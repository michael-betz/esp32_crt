#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "esp_log.h"

#include "http_client.h"

// using esp_http_client
#if defined(ESP_PLATFORM)
	#include "esp_http_client.h"

	static esp_err_t esp_http_cb(esp_http_client_event_t *evt)
	{
		http_on_data_cb cb = (http_on_data_cb)evt->user_data;

		if (evt->event_id == HTTP_EVENT_ON_DATA)
			cb(evt->data, evt->data_len);

		return ESP_OK;
	}

	void http_request(const char *url, http_on_data_cb on_data);
	{
		esp_http_client_config_t config = {
			.url = url,
			// if needed root cert  is not in the bundle:
			// `openssl s_client -showcerts -connect <host>:443`
			// take the last one and add it to the bundle with menuconfig
			.crt_bundle_attach = esp_crt_bundle_attach,
			.event_handler = esp_http_cb,
			.user_data = on_data
		};
		esp_http_client_handle_t client = esp_http_client_init(&config);

		// GET
		err = esp_http_client_perform(client);
		if (err != ESP_OK) {
			ESP_LOGE(T, "HTTP GET request failed: %s", esp_err_to_name(err));
		}
	}
#endif

static lwjson_stream_parser_t stream_parser;

static void parse_json_cb(uint8_t *p, int N)
{
	ESP_LOGD(T, "HTTP_DATA, len=%d", N);

	while (N--) {
		lwjsonr_t res = lwjson_stream_parse(&stream_parser, *p++);
		if (res == lwjsonSTREAMINPROG) {
		} else if (res == lwjsonSTREAMWAITFIRSTCHAR) {
			ESP_LOGI(T, "lwjson_stream_parse waiting for first character");
		} else if (res == lwjsonSTREAMDONE) {
			ESP_LOGI(T, "lwjson_stream_parse done");
		} else {
			ESP_LOGE(T, "lwjson_stream_parse error: %d", res);
		}
	}
}

void http_request_parse_json(const char *url, lwjson_stream_parser_callback_fn evt_fn)
{
	lwjson_stream_init(&stream_parser, evt_fn);
	http_request(url, parse_json_cb);
}
