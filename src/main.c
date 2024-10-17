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
#include "qrcode.h"
#include "driver/gpio.h"
#include "driver/ledc.h"
#include "fonts/font_data.h"

#include "i2s.h"
#include "draw.h"
#include "wireframe_draw.h"
#include "fast_sin.h"
#include "dds.h"
#include "wifi.h"
#include "static_ws.h"
#include "json_settings.h"
#include "encoder.h"
#include "screen_handler.h"
#include "meteo_swiss.h"
#include "main.h"

static const char *T = "MAIN";

char qr_code_str[32];
char *qr_code = NULL;
unsigned qr_code_w = 0;

// callback function to store qr-code in char *qr_code
void esp_qrcode_store(esp_qrcode_handle_t qrcode)
{
	int size = esp_qrcode_get_size(qrcode);
	qr_code_w = size;

	free(qr_code);
	qr_code = malloc(size * size / 8);

	if (qr_code == NULL) {
		printf("!!! esp_qrcode_store malloc failed\n");
		return;
	}

	char *p = qr_code;
	int n_bits = 0;
	unsigned tmp = 0;

	// printf("qr code, width = %d\n\n", size);
	for (int y = 0; y < size; y++) {
		for (int x = 0; x < size; x++) {
			if (esp_qrcode_get_module(qrcode, x, y)) {
				tmp |= 1 << n_bits;
				// printf("O");
			} else {
				// printf(" ");
			}
			if (n_bits++ >= 7) {
				*p++ = tmp;
				tmp = 0;
				n_bits = 0;
			}
		}
		// printf("\n");
	}
}

static void i2s_stream_task(void *args)
{
	i2s_init();
	init_lut();

	while (true)
		screen_handler();

	vTaskDelete(NULL);
}

static void init_io()
{
	gpio_set_level(PIN_HVPS_DIS, 0);
	gpio_set_direction(PIN_HVPS_DIS, GPIO_MODE_OUTPUT);

	// gpio_set_direction(PIN_LED_0, GPIO_MODE_OUTPUT);
	// gpio_set_direction(PIN_LED_1, GPIO_MODE_OUTPUT);
	// gpio_set_direction(PIN_LED_2, GPIO_MODE_OUTPUT);

	// gpio_set_level(PIN_LED_0, 0);
	// gpio_set_level(PIN_LED_1, 0);
	// gpio_set_level(PIN_LED_2, 0);

	gpio_set_direction(PIN_PD_BAD, GPIO_MODE_INPUT);
	gpio_set_direction(PIN_BUTTON, GPIO_MODE_INPUT);
	gpio_set_direction(PIN_WIFI_BUTTON, GPIO_MODE_INPUT);
	gpio_set_direction(PIN_KNOB_A, GPIO_MODE_INPUT);
	gpio_set_direction(PIN_KNOB_B, GPIO_MODE_INPUT);

	gpio_set_pull_mode(PIN_PD_BAD, GPIO_PULLDOWN_ONLY);
	gpio_set_pull_mode(PIN_BUTTON, GPIO_PULLUP_ONLY);
	gpio_set_pull_mode(PIN_KNOB_A, GPIO_PULLUP_ONLY);
	gpio_set_pull_mode(PIN_KNOB_B, GPIO_PULLUP_ONLY);

	// Led PWM
	ledc_timer_config_t ledc_timer = {
		.speed_mode       = LEDC_LOW_SPEED_MODE,
		.timer_num        = LEDC_TIMER_0,
		.duty_resolution  = LEDC_TIMER_12_BIT,
		.freq_hz          = 4000,  // Set output frequency at 4 kHz
		.clk_cfg          = LEDC_AUTO_CLK
	};
	ESP_ERROR_CHECK(ledc_timer_config(&ledc_timer));

	uint8_t pins[] = {PIN_LED_0, PIN_LED_1, PIN_LED_2};

	for (unsigned i=0; i<sizeof(pins); i++) {
		ledc_channel_config_t ledc_channel = {
			.speed_mode     = LEDC_LOW_SPEED_MODE,
			.channel        = i,
			.timer_sel      = LEDC_TIMER_0,
			.intr_type      = LEDC_INTR_DISABLE,
			.gpio_num       = pins[i],
			.duty           = 0, // Set duty to 0%
			.hpoint         = 0
		};
		ESP_ERROR_CHECK(ledc_channel_config(&ledc_channel));
	}

}

// set brightness of led with index 0 .. 2 to val 0 .. 1023
void set_led(unsigned index, unsigned val)
{
	// Set duty to val
	ESP_ERROR_CHECK(ledc_set_duty(LEDC_LOW_SPEED_MODE, index, val));
	// Update duty to apply the new value
	ESP_ERROR_CHECK(ledc_update_duty(LEDC_LOW_SPEED_MODE, index));
}

void update_leds(int i)
{
	if (gpio_get_level(PIN_PD_BAD))
		set_led(0, 0);
	else
		set_led(0, LED_BRIGHTNESS_0);

	switch (wifi_state) {
	case WIFI_NOT_CONNECTED:
		set_led(1, 0);
		break;

	case WIFI_SCANNING:
		// When wifi is trying to connect, make the second LED `breathe`
		set_led(1, (get_sin(i * 500) >> 25) + 64);
		break;

	case WIFI_AP_MODE:
		set_led(1, (get_sin(i * 150) >> 25) + 64);
		break;

	case WIFI_DPP_LISTENING:
		set_led(1, ((i % 20) > 10) * 64);
		break;

	case WIFI_CONNECTED:
		set_led(1, LED_BRIGHTNESS_1);
		break;

	default:
	}
}

void every_second()
{
	static time_t now_ = 0;
	time_t now = 0;
	struct tm timeinfo;

	time(&now);

	if (now != now_) {
		localtime_r(&now, &timeinfo);
		strftime(qr_code_str, sizeof(qr_code_str), "%H:%M:%S", &timeinfo);

		// esp_qrcode_config_t cfg;
		// cfg.qrcode_ecc_level = ESP_QRCODE_ECC_MED;
		// cfg.max_qrcode_version = 40;
		// cfg.display_func = esp_qrcode_store;
		// esp_qrcode_generate(&cfg, qr_code_str);

		now_ = now;
	}
}

void app_main(void)
{
	init_io();
	init_encoder();

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
	static int wifi_state_ = WIFI_NOT_CONNECTED;
	while (1) {
		update_leds(i);

		if (gpio_get_level(PIN_WIFI_BUTTON) == 0) {
			// tryEasyConnect();
			tryApMode();
		}

		// every 2 min
		if ((i % (10 * 60 * 2)) == 0 && wifi_state == WIFI_NOT_CONNECTED)
			tryJsonConnect();

		if (wifi_state == WIFI_CONNECTED) {
			// every hour
			if ((i % (10 * 60 * 60)) == 0) {
				request_weather_data();
			}
		}

		if (wifi_state_ != WIFI_CONNECTED && wifi_state == WIFI_CONNECTED) {  // On new connection
			request_weather_data();
		}

		every_second();

		i++;
		wifi_state_ = wifi_state;

		// ESP_LOGI(T, "Encoder: %d", get_encoder());
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
