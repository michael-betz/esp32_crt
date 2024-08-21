// 40 MHz output without gaps
// outputs 2 bytes with CS low, then 2 bytes with CS high
// invert one of the CS lines so that this can drive 2x MCP4822T DACs

#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_spiffs.h"
#include "esp_log.h"
#include "esp_tls.h"
#include "esp_http_client.h"
#include "esp_crt_bundle.h"
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
#include "encoder.h"
#include "demo_mode.h"
#include "meteo_swiss.h"
#include "main.h"

static const char *T = "MAIN";

#define MAX_HTTP_RECV_BUFFER 20000

esp_err_t _http_event_handler(esp_http_client_event_t *evt)
{
	static int n_received = 0;
	static char *buff = NULL;

	switch(evt->event_id) {
		case HTTP_EVENT_ERROR:
			ESP_LOGE(T, "HTTP_EVENT_ERROR");
        	free(buff);
	    	buff = NULL;
        	n_received = 0;
			break;

		case HTTP_EVENT_ON_CONNECTED:
			ESP_LOGI(T, "HTTP_EVENT_ON_CONNECTED");
			n_received = 0;
			if (buff != NULL)
				free(buff);
			buff = malloc(MAX_HTTP_RECV_BUFFER);
			memset(buff, 0, MAX_HTTP_RECV_BUFFER);
			if (buff == NULL) {
				ESP_LOGE(T, "Couldn't allocate buff");
				return ESP_FAIL;
			}
			break;

		case HTTP_EVENT_HEADER_SENT:
			ESP_LOGI(T, "HTTP_EVENT_HEADER_SENT");
			break;

		case HTTP_EVENT_ON_HEADER:
			ESP_LOGI(T, "HTTP_EVENT_ON_HEADER, key=%s, value=%s", evt->header_key, evt->header_value);
			break;

		case HTTP_EVENT_ON_DATA:
			ESP_LOGI(T, "HTTP_EVENT_ON_DATA, len=%d", evt->data_len);
			if (n_received == 0) {
				bool is_chunked = esp_http_client_is_chunked_response(evt->client);
				int content_len = esp_http_client_get_content_length(evt->client);
				printf("is_chunked: %d, content_len: %d\n", is_chunked, content_len);
			}
			if (n_received + evt->data_len > MAX_HTTP_RECV_BUFFER) {
				ESP_LOGE(T, "HTTP_EVENT_ON_DATA buff overflow");
				free(buff);
				buff = NULL;
				n_received = 0;
				return ESP_FAIL;
			}
			memcpy(buff + n_received, evt->data, evt->data_len);
			n_received += evt->data_len;
			break;

		case HTTP_EVENT_ON_FINISH:
			ESP_LOGI(T, "HTTP_EVENT_ON_FINISH, n_received = %d", n_received);
			// fwrite(buff, n_received, 1, stdout);

			// parse the json response
			cJSON *j = cJSON_ParseWithLength(buff, n_received);
			if (j == NULL) {
				const char *p = cJSON_GetErrorPtr();
				ESP_LOGE(T, "get_weather(): HTTP_EVENT_ON_FINISH, JSON Error at char %d", p - buff);
			} else {
				weather_set_json(j);
			}

			free(buff);
			buff = NULL;
			n_received = 0;
			break;

		case HTTP_EVENT_DISCONNECTED:
			ESP_LOGI(T, "HTTP_EVENT_DISCONNECTED");
			int mbedtls_err = 0;
			esp_err_t err = esp_tls_get_and_clear_last_error((esp_tls_error_handle_t)evt->data, &mbedtls_err, NULL);
			if (err != 0) {
				ESP_LOGI(T, "Last esp error code: 0x%x", err);
				ESP_LOGI(T, "Last mbedtls failure: 0x%x", mbedtls_err);
			}
        	free(buff);
	    	buff = NULL;
        	n_received = 0;
			break;

		case HTTP_EVENT_REDIRECT:
			ESP_LOGI(T, "HTTP_EVENT_REDIRECT");
			break;

	}
	return ESP_OK;
}


static void get_weather()
{
	esp_err_t err;
	const char url[] = "https://app-prod-ws.meteoswiss-app.ch/v1/plzDetail?plz=120200";

	esp_http_client_config_t config = {
		.url = url,
		// if needed root cert  is not in the bundle:
		// `openssl s_client -showcerts -connect <host>:443`
		// take the last one and add it to the bundle with menuconfig
		.crt_bundle_attach = esp_crt_bundle_attach,
		.event_handler = _http_event_handler
	};
	esp_http_client_handle_t client = esp_http_client_init(&config);

	// GET
	err = esp_http_client_perform(client);
	if (err != ESP_OK) {
		ESP_LOGE(T, "HTTP GET request failed: %s", esp_err_to_name(err));
	}
}

static void i2s_stream_task(void *args)
{
	i2s_init();
	init_lut();

	while (true)
		demo_mode();

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
	gpio_set_direction(PIN_KNOB_A, GPIO_MODE_INPUT);
	gpio_set_direction(PIN_KNOB_B, GPIO_MODE_INPUT);

	gpio_set_pull_mode(PIN_BUTTON, GPIO_PULLUP_ONLY);
	gpio_set_pull_mode(PIN_KNOB_A, GPIO_PULLUP_ONLY);
	gpio_set_pull_mode(PIN_KNOB_B, GPIO_PULLUP_ONLY);
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
		gpio_set_level(PIN_LED_B, (i % 20) == 0);
		// gpio_set_level(PIN_LED_G, gpio_get_level(PIN_KNOB_A));
		// gpio_set_level(PIN_LED_B, gpio_get_level(PIN_KNOB_B));

		if ((i % (10 * 60 * 2)) == 0 && wifi_state == WIFI_NOT_CONNECTED)  // every 2 min
			tryJsonConnect();

		if (wifi_state == WIFI_CONNECTED) {
			if ((i % (10 * 60 * 60)) == 0) {  // every hour
				get_weather();
			}
		}

		if (wifi_state_ != WIFI_CONNECTED && wifi_state == WIFI_CONNECTED) {  // On new connection
			get_weather();
		}

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
