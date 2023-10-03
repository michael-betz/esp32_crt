#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/spi_master.h"
#include "esp_task_wdt.h"

void app_main(void)
{
	printf("Hello, this is an SPI - test!\n");

	esp_err_t ret;
	spi_device_handle_t handle;
	spi_bus_config_t buscfg = {  // SPI2, SPI3 IO pins
		.mosi_io_num = 13,  // 13, 23
		.sclk_io_num = 14,  // 14, 18
		.quadwp_io_num = -1,
		.quadhd_io_num = -1,
		.max_transfer_sz = 0x20000  // 131 kB
	};
	spi_device_interface_config_t devcfg = {
		.flags = SPI_DEVICE_NO_DUMMY,
		.command_bits = 0,
		.address_bits = 0,
		.dummy_bits = 0,
		.mode = 0,                                // SPI mode 0
		.clock_speed_hz = SPI_MASTER_FREQ_40M,    // Clock frequency
		.spics_io_num = 15,               		  // CS pin: 15, 5
		.queue_size = 1                           // We want to be able to queue N transactions at a time
	};
	// Initialize the SPI bus
	ret = spi_bus_initialize(SPI2_HOST, &buscfg, SPI_DMA_CH_AUTO);
	ESP_ERROR_CHECK(ret);
	// Attach the Device to the SPI bus
	ret = spi_bus_add_device(SPI2_HOST, &devcfg, &handle);
	ESP_ERROR_CHECK(ret);

	#define BUF_SIZE 0x10000  // 65 kB, 13 ms at 40 MHz
	uint8_t *buffer_a = heap_caps_malloc(BUF_SIZE, MALLOC_CAP_DMA);
	assert(buffer_a);
	uint8_t *buffer_b = heap_caps_malloc(BUF_SIZE, MALLOC_CAP_DMA);
	assert(buffer_b);

	ret = spi_device_acquire_bus(handle, portMAX_DELAY);
	ESP_ERROR_CHECK(ret);

	printf("SPI init succeeded\n");

	spi_transaction_t trans_desc = {
		.length = BUF_SIZE * 8,
		.rxlength = 0,
		.tx_buffer = buffer_a,
		.rx_buffer = NULL
	};

	unsigned cnt = 0;
	while (1) {
		// Transmit new buffer in background
		ret = spi_device_polling_start(handle, &trans_desc, portMAX_DELAY);
		ESP_ERROR_CHECK(ret);

		// Fill the other buffer while transmit is in progress
		uint8_t *dest;
		if (trans_desc.tx_buffer == buffer_a) {
			dest = buffer_b;
		} else {
			dest = buffer_a;
		}
		for (unsigned i = 0; i < BUF_SIZE; i++)
			dest[i] = i + cnt;
		printf("*");
		fflush(stdout);
		// We have a CPU budget of around 10 ms to fill the next buffer
		vTaskDelay(5 / portTICK_PERIOD_MS);

		// Wait until transmit finished
		spi_device_polling_end(handle, portMAX_DELAY);

		// Transmit the other buffer next
		trans_desc.tx_buffer = dest;

		cnt++;
	}
}
