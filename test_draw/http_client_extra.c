#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "http_client.h"

#if defined(__EMSCRIPTEN__)
// using emscripten async_wget. No good for fetching from external APIs as it gets blocked by CORS :(
	#include "emscripten.h"
	static void async_wget_data(void *user_data, void *data, int len)
	{
		http_on_data_cb cb = (http_on_data_cb)user_data;
		cb(data, len);
	}

	static void async_wget_err(void *user_data)
	{
		printf("Failed to do async_wget request :(\n");
	}

	void http_request(const char *url, http_on_data_cb on_data)
	{
		emscripten_async_wget_data(url, on_data, async_wget_data, async_wget_err);
	}
#else
// using libcurl
	#include <curl/curl.h>

	static size_t curl_cb(void *contents, size_t size, size_t nmemb, void *userp)
	{
		http_on_data_cb cb = (http_on_data_cb)userp;
		cb(contents, size * nmemb);
		return size * nmemb;
	}

	void http_request(const char *url, http_on_data_cb on_data)
	{
		CURL *curl = curl_easy_init();
		curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, curl_cb);
		curl_easy_setopt(curl, CURLOPT_URL, url);
		curl_easy_setopt(curl, CURLOPT_WRITEDATA, on_data);
		CURLcode res = curl_easy_perform(curl);

		if(res != CURLE_OK) {
			  printf("curl_easy_perform() failed: %s\n", curl_easy_strerror(res));
		}
	}
#endif
