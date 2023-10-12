#include <stdint.h>
#include <stdlib.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "i2s.h"


// 40 MHz output without gaps
// outputs 2 bytes with CS low, then 2 bytes with CS high
// invert one of the CS lines so that this can drive 2x MCP4822T DACs


static void i2s_stream_task(void *args)
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

	xTaskCreate(i2s_stream_task, "i2s_stream_task", 4096, NULL, 5, NULL);
}
