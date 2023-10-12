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
// outputs 2 bytes with CS low, then 2 bytes with CS high
// invert one of the CS lines so that this can drive 2x MCP4822T DACs


static void i2s_example_write_task(void *args)
{
	while (1) {
		i2s_write_chunk();
	}
	vTaskDelete(NULL);
}


void app_main(void)
{
	printf("Hello, this is an SPI - test, I2S version!\n");

	i2s_init();

	/* Step 3: Create writing and reading task, enable and start the channels */
	xTaskCreate(i2s_example_write_task, "i2s_example_write_task", 4096, NULL, 5, NULL);
}
