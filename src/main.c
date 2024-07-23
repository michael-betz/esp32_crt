// 40 MHz output without gaps
// outputs 2 bytes with CS low, then 2 bytes with CS high
// invert one of the CS lines so that this can drive 2x MCP4822T DACs

#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_spiffs.h"
#include "esp_log.h"
#include "driver/gpio.h"
#include "fonts/font_data.h"

#include "i2s.h"
#include "draw.h"
#include "wireframe_draw.h"
#include "fast_sin.h"
#include "dds.h"
#include "wifi.h"
#include "static_ws.h"
#include "json_settings.h"
#include "main.h"

static const char *T = "MAIN";


static void demo_text(unsigned font)
{
	char tmp_str[64];
	static int frame = 0;

	if (font >= N_FONTS)
		return;
	snprintf(tmp_str, sizeof(tmp_str), "f: %d", font);

	set_font(0);
	push_str(
		-1300, 1300,
		tmp_str,
		sizeof(tmp_str),
		A_LEFT,
		300,
		200
	);

	time_t now;
	struct tm timeinfo;

	time(&now);
	localtime_r(&now, &timeinfo);
    strftime(tmp_str, sizeof(tmp_str), "%A\n%d.%m.%y\n%k:%M:%S", &timeinfo);

	int font_size = ((get_sin(frame++ * MAX_ANGLE / 5000) >> 16) + (1 << 15)) * 1000 / (1 << 16) + 50;
	set_font(font);
	push_str(
		0, 500,
		tmp_str,
		sizeof(tmp_str),
		A_CENTER,
		font_size,
		300
	);
}

static void test_image()
{
	// a square around the screen
	push_goto(-2000, -2000);
	push_line(-2000, 2000, 30);
	push_line(2000, 2000, 30);
	push_line(2000, -2000, 30);
	push_line(-2000, -2000, 30);

	// inner cross
	push_goto(-200, -200);
	push_line(200, 200, 255);
	push_goto(-200, 200);
	push_line(200, -200, 255);

	// inner +
	push_goto(-500, 0);
	push_line(500, 0, 50);
	push_goto(0, 500);
	push_line(0, -500, 50);

	// concentric circles
	for (unsigned i=1; i<=10; i++) {
		push_circle(
			0,
			0,
			i * 200,
			i * 200,
			i <= 5 ? 0 : -280,
			i <= 5 ? MAX_ANGLE : MAX_ANGLE - 1500,
			100
		);
	}

	time_t now;
	struct tm timeinfo;
	char tmp_str[16];

	time(&now);
	localtime_r(&now, &timeinfo);
    strftime(tmp_str, sizeof(tmp_str), "%k:%M:%S", &timeinfo);

	set_font(0);
	push_str(0, -1800, tmp_str, sizeof(tmp_str), A_CENTER, 900, 400);
}

static void i2s_stream_task(void *args)
{
	i2s_init();
	init_lut();
	TickType_t ticks;
	int demo_text_font = 0;

	while (1) {
		// ticks = xTaskGetTickCount();
		// while ((xTaskGetTickCount() - ticks) < pdMS_TO_TICKS(10000))
		// 	test_image();

		// ticks = xTaskGetTickCount();
		// while ((xTaskGetTickCount() - ticks) < pdMS_TO_TICKS(60000))
		// 	wf_test();

		// ticks = xTaskGetTickCount();
		// demo_text_font = random() % N_FONTS;
		// while ((xTaskGetTickCount() - ticks) < pdMS_TO_TICKS(60000))
		// 	demo_text(demo_text_font);

		ticks = xTaskGetTickCount();
		while ((xTaskGetTickCount() - ticks) < pdMS_TO_TICKS(60000)) {
			draw_dds(100000);
			// nudge_dds();
		}
	}

	vTaskDelete(NULL);
}

static void init_io()
{
	gpio_set_level(PIN_HVPS_DIS, 0);
	gpio_set_direction(PIN_HVPS_DIS, GPIO_MODE_OUTPUT);

	gpio_set_direction(PIN_LED_R, GPIO_MODE_OUTPUT);
	gpio_set_direction(PIN_LED_G, GPIO_MODE_OUTPUT);
	gpio_set_direction(PIN_LED_B, GPIO_MODE_OUTPUT);

	gpio_set_level(PIN_LED_R, 0);
	gpio_set_level(PIN_LED_G, 0);
	gpio_set_level(PIN_LED_B, 0);

	gpio_set_direction(PIN_PD_BAD, GPIO_MODE_INPUT);
	gpio_set_direction(PIN_BUTTON, GPIO_MODE_INPUT);
}

void app_main(void)
{
	init_io();

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

	ESP_LOGI(T, "Hello, this is %s, I2S version!", jGetS(getSettings(), "hostname", WIFI_HOST_NAME));

	initWifi();

	// Using priority 19 or higher guarantees that an application task
	// can run on Core 1 without being preempted by any built-in task.
	// https://docs.espressif.com/projects/esp-idf/en/latest/esp32/api-guides/performance/speed.html#choosing-task-priorities-of-the-application
	xTaskCreatePinnedToCore(i2s_stream_task, "i2s_stream_task", 4096, NULL, 22, NULL, 1);

	int i = 0;
	while (1) {
		// gpio_set_level(PIN_LED_R, !gpio_get_level(PIN_HVPS_DIS));
		gpio_set_level(PIN_LED_G, (i % 10) == 0);
		// gpio_set_level(PIN_LED_B, gpio_get_level(PIN_PD_BAD));

		if ((i % 1200) == 0 && wifi_state == WIFI_NOT_CONNECTED)  // every 2 min
			tryJsonConnect();

		i++;
		vTaskDelay(100 / portTICK_PERIOD_MS);
	}
}

void ws_callback(uint8_t *payload, unsigned len)
{
	char *tok = NULL;
	unsigned args[5];

	ESP_LOGI(T, "ws_callback(%d)", len);
	if (len < 1)
		return;

	switch (payload[0]) {
	case 'd':
			char *p_tmp = (char*)(&payload[2]);
			for(unsigned i = 0; i < 5; i++) {
				tok = strsep(&p_tmp, ",");
				if (tok == NULL) {
					ESP_LOGE(T, "parse error!");
					ESP_LOG_BUFFER_HEXDUMP(T, payload, len, ESP_LOG_ERROR);
					return;
				}
				args[i] = strtoul(tok, NULL, 16);
			}
			setup_dds(args[0], args[1], args[2], args[3], args[4]);
		break;
	}
}
