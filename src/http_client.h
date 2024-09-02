#pragma once
#include "lwjson.h"

// Provide a way to do http GET requests. This should work on all platforms
// (SDL, emscripten, esp32) in the same way. Also provide a convenient function
// to parse json while the request is received.

typedef void (*http_on_data_cb)(uint8_t *buffer, int len);

void http_request(const char *url, http_on_data_cb on_data);

// Use this for streaming the GET response and decoding the .json on the fly
// (to minimize ram usage)
void http_request_parse_json(const char *url, lwjson_stream_parser_callback_fn evt_fn);

// Use this for opening a file and decoding the .json on the fly
// (to minimize ram usage)
void file_parse_json(const char *filename, lwjson_stream_parser_callback_fn evt_fn);
