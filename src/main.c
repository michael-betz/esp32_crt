// 40 MHz output without gaps
// outputs 2 bytes with CS low, then 2 bytes with CS high
// invert one of the CS lines so that this can drive 2x MCP4822T DACs

#include <stdint.h>
#include <stdlib.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_spiffs.h"
#include "esp_log.h"

#include "i2s.h"
#include "draw.h"
#include "fast_sin.h"
#include "dds.h"
#include "wifi.h"
#include "static_ws.h"
#include "json_settings.h"
#include "print.h"

static const char *T = "MAIN";

#define SAMPLES_PER_FRAME 625000 / 100
unsigned char dl_test[] = {
  0x02, 0x64, 0x06, 0x00, 0x00, 0x80, 0x00, 0x80, 0x18, 0xfc, 0x18, 0xfc,
  0x18, 0xfc, 0xe8, 0x03, 0xe8, 0x03, 0xe8, 0x03, 0xe8, 0x03, 0x18, 0xfc,
  0x18, 0xfc, 0x18, 0xfc, 0x03, 0x64, 0x00, 0x00, 0x00, 0x00, 0xbc, 0x02,
  0x00, 0x00, 0xd0, 0xdc, 0x03, 0x64, 0x00, 0x00, 0x00, 0x00, 0x20, 0x03,
  0x00, 0x00, 0xd0, 0xdc, 0x03, 0x64, 0x00, 0x00, 0x00, 0x00, 0x84, 0x03,
  0x00, 0x00, 0xd0, 0xdc, 0x05, 0xc8, 0x20, 0x03, 0x00, 0x00, 0x90, 0x01,
  0x0d, 0x0b, 0x48, 0x65, 0x6c, 0x6c, 0x6f, 0x20, 0x57, 0x6f, 0x72, 0x6c,
  0x64, 0x05, 0xc8, 0x20, 0x03, 0x00, 0x00, 0x70, 0xfe, 0x0e, 0x0d, 0x41,
  0x6e, 0x6f, 0x74, 0x68, 0x65, 0x72, 0x20, 0x66, 0x6f, 0x6e, 0x74, 0x21,
  0xfe
};
unsigned int dl_test_len = 109;



static void i2s_stream_task(void *args)
{
	i2s_init();
	init_lut();
	while (1) {
		push_list(dl_test, sizeof(dl_test));
		draw_dds(10000);
	}
	vTaskDelete(NULL);
}

void app_main(void)
{
	printf("Hello, this is esp32_crt, I2S version!\n");

	// Mount spiffs for *.html and defaults.json
	esp_vfs_spiffs_conf_t conf = {
		.base_path = "/spiffs",
		.partition_label = NULL,
		.max_files = 4,
		.format_if_mount_failed = true
	};
	esp_vfs_spiffs_register(&conf);

	// Load settings.json from SPIFFS, try to create file if it doesn't exist
	set_settings_file("/spiffs/settings.json", "/spiffs/default_settings.json");

	// initWifi();
	// tryConnect();

	// This makes it quite easy for the application to place high priority tasks
	// on Core 1. Using priority 19 or higher guarantees that an application task
	// can run on Core 1 without being preempted by any built-in task.
	// https://docs.espressif.com/projects/esp-idf/en/latest/esp32/api-guides/performance/speed.html#choosing-task-priorities-of-the-application
	xTaskCreatePinnedToCore(i2s_stream_task, "i2s_stream_task", 4096, NULL, 22, NULL, 1);
}

void _putchar(char c){
	putchar(c);
}

void ws_callback(uint8_t *payload, unsigned len)
{
	char *tok = NULL;
	unsigned args[5];

	log_i("ws_callback(%d)", len);
	if (len < 1)
		return;

	hexDump(payload, len);

	switch (payload[0]) {
	case 'd':
			for(unsigned i = 0; i < 6; i++) {
				tok = strsep(&payload, ",");
				if (tok == NULL) {
					log_e("parse error!\n");
					return;
				}
				if (i >= 1)
					args[i] = strtoul(tok, NULL, 16);
			}
			setup_dds(args[0], args[1], args[2], args[3], args[4]);
		break;
	}
}
