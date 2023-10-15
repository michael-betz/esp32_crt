// 40 MHz output without gaps
// outputs 2 bytes with CS low, then 2 bytes with CS high
// invert one of the CS lines so that this can drive 2x MCP4822T DACs

#include <stdint.h>
#include <stdlib.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "i2s.h"
#include "draw.h"

#define SAMPLES_PER_FRAME 625000 / 100

static draw_list_t dl_test[] = {
	{ 0 << FP, 2000 << FP, 1},
	{2000 << FP, 2000 << FP, 1},
	{2000 << FP,  0 << FP, 1},
	{ 0 << FP,  0 << FP, 1},
	{900 << FP, 900 << FP, 0},
	{1100 << FP, 1100 << FP, 128},
	{900 << FP, 1100 << FP, 0},
	{1100 << FP, 900 << FP, 128},
	{ 0 << FP,  0 << FP, 0},
};
static const unsigned n_dl_test = sizeof(dl_test) / sizeof(dl_test[0]);

static void i2s_stream_task(void *args)
{
	while (1) {
		unsigned n_samples = push_list(dl_test, n_dl_test);
		printf("%d ", n_samples);

		// zero-padding to get fixed number of samples per frame
		while (n_samples++ < SAMPLES_PER_FRAME) {
			push_sample(0xF000, 0, 0, 0);
		}

		fflush(stdout);
	}
	vTaskDelete(NULL);
}

void app_main(void)
{
	printf("Hello, this is an SPI - test, I2S version!\n");

	i2s_init();

	xTaskCreate(i2s_stream_task, "i2s_stream_task", 4096, NULL, 5, NULL);
}

void _putchar(char c){
	putchar(c);
}
