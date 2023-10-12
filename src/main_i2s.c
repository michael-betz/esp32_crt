#include <stdint.h>
#include <stdlib.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/i2s_std.h"
#include "driver/gpio.h"
#include "esp_check.h"
#include "sdkconfig.h"
#include "soc/i2s_struct.h"
#include "soc/rtc.h"

#include "soc/io_mux_reg.h"
#include "hal/gpio_hal.h"


// 40 MHz output without gaps
// outputs 4 bytes with CS low, then 4 bytes with CS high
// invert one of the CS lines so that this can drive 2x MCP4822T DACs

#define I2S I2S_NUM_1
#define EXAMPLE_BUFF_SIZE               1024 * 2 * 4

#define A_N 15
#define GA_N 13
#define SHDN_N 12

static i2s_chan_handle_t                tx_chan;        // I2S tx channel handler

static void i2s_example_write_task(void *args)
{
	unsigned cnt = 0;
	uint16_t *w_buf = (uint16_t *)calloc(1, EXAMPLE_BUFF_SIZE);
	assert(w_buf);  // Check if w_buf allocation success

	size_t w_bytes = EXAMPLE_BUFF_SIZE;

	/* (Optional) Preload the data before enabling the TX channel, so that the valid data can be transmitted immediately */
	// while (w_bytes == EXAMPLE_BUFF_SIZE) {
	// 	/* Here we load the target buffer repeatedly, until all the DMA buffers are preloaded */
	// 	ESP_ERROR_CHECK(i2s_channel_preload_data(tx_chan, w_buf, EXAMPLE_BUFF_SIZE, &w_bytes));
	// }

	/* Enable the TX channel */
	ESP_ERROR_CHECK(i2s_channel_enable(tx_chan));
	while (1) {
		/* Write i2s data */
		if (i2s_channel_write(tx_chan, w_buf, EXAMPLE_BUFF_SIZE, &w_bytes, 1000) == ESP_OK) {
			printf("*");
			fflush(stdout);

			/* Assign w_buf */
			for (int i = 0; i < EXAMPLE_BUFF_SIZE / 2; i += 4) {
				// sample values of the 4 channels
				uint16_t val_a = cnt & 0x0FFF;
				uint16_t val_b = 0;
				uint16_t val_c = 0x0FFF - val_a;
				uint16_t val_d = 0;

				// output the sample-data for 4 channels over the next 64 clocks
				w_buf[i] = 		(0 << A_N) | (0 << GA_N) | (1 << SHDN_N) | val_a;  // /CS0
				w_buf[i + 1] = 	(0 << A_N) | (0 << GA_N) | (0 << SHDN_N) | val_b;  // /CS1
				w_buf[i + 2] = 	(1 << A_N) | (0 << GA_N) | (1 << SHDN_N) | val_c;  // /CS0
				w_buf[i + 3] =  (1 << A_N) | (0 << GA_N) | (0 << SHDN_N) | val_d;  // /CS1

				cnt++;
			}

		} else {
			printf("Write Task: i2s write failed\n");
		}
		// vTaskDelay(1);
	}
	free(w_buf);
	vTaskDelete(NULL);
}

static void i2s_example_init_std_simplex(void)
{
	i2s_chan_config_t tx_chan_cfg = {
		.id = I2S,
		.role = I2S_ROLE_MASTER,
		.dma_desc_num = 6,
		.dma_frame_num = 240,
		.auto_clear = true,
	};
	ESP_ERROR_CHECK(i2s_new_channel(&tx_chan_cfg, &tx_chan, NULL));

	i2s_std_config_t tx_std_cfg = {
		.clk_cfg  = {
			.sample_rate_hz = 200000,  // not relevant
			.clk_src = SOC_MOD_CLK_PLL_F160M,
			// .clk_src = SOC_MOD_CLK_APLL,
			.mclk_multiple = I2S_MCLK_MULTIPLE_128
		},
		.slot_cfg = {
			.data_bit_width = I2S_DATA_BIT_WIDTH_16BIT,
			.slot_bit_width = I2S_SLOT_BIT_WIDTH_AUTO,
			.slot_mode = I2S_SLOT_MODE_STEREO,
			.slot_mask = I2S_STD_SLOT_BOTH,
			.ws_width = 32,
			.ws_pol = false,
			.bit_shift = false,
			.msb_right = true
		},
		.gpio_cfg = {
			.mclk = I2S_GPIO_UNUSED,    // some codecs may require mclk signal, this example doesn't need it
			.bclk = 14,
			.ws   = 15,  // This is used as /CS0
			.dout = 13,
			.din  = -1,
			.invert_flags = {
				.mclk_inv = false,
				.bclk_inv = false,
				.ws_inv   = false,
			},
		},
	};
	ESP_ERROR_CHECK(i2s_channel_init_std_mode(tx_chan, &tx_std_cfg));

	// rtc_clk_apll_coeff_set(0, 0, 0, 9);  // Fastest possible APLL clock
	// I2S1.clkm_conf.clka_en = 1;  // enable APLL clock

	// This produce a 40 MHz bit-clock
	I2S1.clkm_conf.clk_en = 1;
	I2S1.clkm_conf.clkm_div_num = 2;  // integral divider, min is 2
	I2S1.clkm_conf.clkm_div_a = 1;  // fractional divider numerator
	I2S1.clkm_conf.clkm_div_b = 0;  // fractional divider denominator
	I2S1.sample_rate_conf.tx_bck_div_num = 2;  // bit-clock divider, min is 2
	// I2S1.sample_rate_conf.tx_bits_mod = 16;  // bit length of transmitter channel

	// Output an additional inverted WS on GPIO2
	// This is used as /CS1
	gpio_hal_iomux_func_sel(IO_MUX_GPIO2_REG, PIN_FUNC_GPIO);
	gpio_set_direction(GPIO_NUM_2, GPIO_MODE_OUTPUT);
	esp_rom_gpio_connect_out_signal(GPIO_NUM_2, I2S1O_WS_OUT_IDX, true, 0);
}

void app_main(void)
{
	printf("Hello, this is an SPI - test, I2S version!\n");

	i2s_example_init_std_simplex();

	/* Step 3: Create writing and reading task, enable and start the channels */
	xTaskCreate(i2s_example_write_task, "i2s_example_write_task", 4096, NULL, 5, NULL);
}
