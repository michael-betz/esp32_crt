#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <time.h>
#include <math.h>
#include <sys/param.h>
#include "meteo_swiss.h"
#include "draw.h"
#include "fonts/font_data.h"
#include "esp_http_client.h"
#include "esp_log.h"
#include "lwjson.h"

static const char *T = "MSWISS";

#define WEATHER_DATA_N 1024
#define GRAPHS_N 8
#define PLOT_WIDTH 3000

typedef struct {
	const char *key;
	uint16_t len;
	int16_t *data;
	int16_t min;
	int16_t max;
} graph_array_t;

typedef struct {
	unsigned graph_start;
	unsigned graph_start_low;
	graph_array_t graphs[GRAPHS_N];
	int16_t weather_data[WEATHER_DATA_N];
} meteo_graph_t;


static meteo_graph_t meteo_graphs = {
	.graphs = {
		{.key = "weatherIcon3h"},
		{.key = "precipitation10m"},
		{.key = "precipitationMax10m"},
		{.key = "precipitation1h"},
		{.key = "precipitationMax1h"},
		{.key = "temperatureMin1h"},
		{.key = "temperatureMean1h"},
		{.key = "temperatureMax1h"},
	}
};

// For access to .graphs by name
enum {
	weatherIcon3h,
	precipitation10m,
	precipitationMax10m,
	precipitation1h,
	precipitationMax1h,
	temperatureMin1h,
	temperatureMean1h,
	temperatureMax1h
} meteo_keys;


// see dev/2022-02-14-Wetter-Icons-inkl-beschreibung-v1-an-website.xlsx
static const uint8_t weather_icon_map_a[] = { //  this is for code 1 - 42 (day)
	0x0d, 0x02, 0x02, 0x0c, 0x13, 0x08, 0x06, 0x65, 0x08,
	0x0a, 0x0a, 0x05, 0x05, 0x1c, 0x17, 0x1b, 0x19, 0xb5,
	0x1b, 0x19, 0xb5, 0x1b, 0x1d, 0x1e, 0x10, 0x7d, 0x03,
	0x14, 0x09, 0x0a, 0x09, 0x09, 0x07, 0x65, 0x41, 0x6b,
	0x6b, 0x0e, 0x0e, 0x05, 0x05, 0x6b
};

static const uint8_t weather_icon_map_b[] = { //  this is for code 101 - 142 (night)
	0x2e, 0x81, 0x86, 0x31, 0x13, 0x29, 0xb4, 0x67,	0x2b,
	0x2b, 0x38, 0x25, 0x25, 0x1c, 0x17, 0x1b, 0x19, 0xb5,
	0x1b, 0x19, 0xb5, 0x1b, 0x1d, 0x1e, 0x1e, 0x80, 0x4a,
	0x14, 0x2b, 0x2a, 0x26, 0x29, 0x28, 0x38, 0x41, 0x25,
	0x69, 0x3a, 0x6c, 0x33, 0x33, 0x6c
};

// Get unicode of matching weather icon from meteo-swiss key
static unsigned cp_from_meteo_swiss_key(unsigned key)
{
	if (key < 1)
		return '?';

	if (key <= 42)
		return 0xf000 + weather_icon_map_a[key - 1];

	if (key < 101)
		return '?';

	if (key <= 142)
		return 0xf000 + weather_icon_map_b[key - 101];

	return '?';
}

static int draw_plot(float x_offset, float dx, float max_x, int y_offset, float dy, unsigned graph_index, int16_t min_val, int density)
{
	if (graph_index >= GRAPHS_N)
		return x_offset;

	graph_array_t *g = &meteo_graphs.graphs[graph_index];

	if (g == NULL || g->len <= 0)
		return x_offset;

	float x = x_offset;

	uint16_t len = g->len;
	int16_t *p = g->data;
	while (len--) {
		if (x > max_x)
			break;

		int y = (*p - min_val) / 100.0f * dy + y_offset;

		if (x == x_offset)
			push_goto(x, y);
		else
			push_line(x, y, density);

		x += dx;
		p++;
	}
	// printf("%s plotted %d points\n", key, i);

	return x;
}

// We fit the max. number of ticks with a min. distance between them from x_offset to max_x
static void draw_plot_x_axis(int x_offset, float x_scale, float max_x, int y_offset)
{
	// Scale needs to go from x_min to x_max in [pixels]
	// time-value in [h] at any position is sample_number / x_scale / 6.0
	// the distance from x_min to x_max shall be divided in a number of N ticks
	// the increment between ticks in [h] shall be an integer number

	x_scale *= 6.0f;

	int actual_increment = ceilf(200.0f / x_scale);  // min increment, rounded up to the nearest integer [h]

	// tick increments shall be divisible by 24 g. Stupid solution but works
	switch (actual_increment) {
	case 5:
		actual_increment = 6;
		break;
	case 7:
		actual_increment = 8;
		break;
	case 9:
	case 10:
	case 11:
		actual_increment = 12;
		break;
	}

	float actual_dist = actual_increment * x_scale;  // distance between ticks [pixels]

	char label[32];
	set_font_name(NULL);

	int hours = 0;

	// push_str(x_offset + actual_dist / 2, y_offset, "[h]", 3, A_CENTER, 150, 100);

	for (float x = x_offset; x < max_x; x += actual_dist) {
		push_goto(x, y_offset);
		push_line(x, y_offset - 75, 100);

		snprintf(label, sizeof(label), "%d", hours % 24);
		push_str(x, y_offset - 300, label, sizeof(label), A_CENTER, 200, 300);
		hours += actual_increment;
	}

	// Mark the 24 h with longer lines
	for (float x = x_offset + 24.0f * x_scale; x < max_x; x += 24.0f * x_scale) {
		push_goto(x, y_offset);
		push_line(x, y_offset + 2500, 20);
	}
}

// We put a tick and label at min_val and max_val only
static void draw_plot_y_axis(int x_offset, int y_offset, float dy, int16_t min_val, int16_t max_val)
{
	char label[32];
	int16_t ticks[] = {min_val, max_val};

	for (int i=0; i<2; i++) {
		int16_t val = ticks[i];
		float y = y_offset + (val - min_val) / 100.0f * dy;

		push_goto(x_offset - 75, y);
		push_line(x_offset, y, 100);

		// Avoid overlapping digits in the axis ticks
		if (i > 0 && y < (y_offset + 180))
			y = y_offset + 180;

		set_font_name(NULL);
		snprintf(label, sizeof(label), "%.1f", val / 100.0f);

		push_str(x_offset - 75, y, label, sizeof(label), A_RIGHT, 200, 200);
	}
}

void rain_temp_plot(unsigned zoom)
{
	int x_end = 0;

	zoom++;  // lowest zoom level shall be 1.0

	// distance between 10m samples [pixel / sample]
	// for x_scale = 1.0  a sample of 10 min corresponds to one 1 pixel
	// for x_scale = 10.0  a sample of 10 min corresponds to 10 pixels
	const float x_scale = 4.0f * zoom;
	const float y_scale = 60.0f;
	const float x_min = -1500.0f;  // pixels
	const float x_max = 1900.0f;  // pixels

	if (meteo_graphs.graphs[temperatureMean1h].len <= 0) {
		set_font_name(NULL);
		push_str(0, 0, "No\nweather\ndata", 17, A_CENTER, 600, 200);
		return;
	}

	int16_t rain_min = MIN(meteo_graphs.graphs[precipitation10m].min, meteo_graphs.graphs[precipitation1h].min);
	int16_t rain_max = MAX(meteo_graphs.graphs[precipitationMax10m].max, meteo_graphs.graphs[precipitationMax1h].max);

	int16_t temp_min = meteo_graphs.graphs[temperatureMin1h].min;
	int16_t temp_max = meteo_graphs.graphs[temperatureMax1h].max;

	// Rain plot
	const int y_offset_a = 400;
	push_str(-1500, y_offset_a + 1000, "[mm/h]", 6, A_LEFT, 250, 150);
	draw_plot_y_axis(x_min - 25.0f, y_offset_a, y_scale, rain_min, rain_max);

	x_end = draw_plot(x_min, x_scale, x_max, y_offset_a, y_scale, precipitation10m, rain_min, 200);
	draw_plot(x_end, x_scale * 6.0f, x_max, y_offset_a, y_scale, precipitation1h, rain_min, 200);

	x_end = draw_plot(x_min, x_scale, x_max, y_offset_a, y_scale, precipitationMax10m, rain_min, 50);
	draw_plot(x_end, x_scale * 6.0f, x_max, y_offset_a, y_scale, precipitationMax1h, rain_min, 50);

	// Temperature plot
	const int y_offset_b = -1100;
	push_str(-1500, y_offset_b + 1000, "[°C]", 5, A_LEFT, 250, 200);
	draw_plot_y_axis(x_min - 25.0f, y_offset_b, y_scale, temp_min, temp_max);
	draw_plot(x_min, x_scale * 6.0f, x_max, y_offset_b, y_scale, temperatureMax1h, temp_min, 50);
	draw_plot(x_min, x_scale * 6.0f, x_max, y_offset_b, y_scale, temperatureMean1h, temp_min, 200);
	draw_plot(x_min, x_scale * 6.0f, x_max, y_offset_b, y_scale, temperatureMin1h, temp_min, 50);

	// x-axis: Try to fit a good number of ticks
	draw_plot_x_axis(x_min, x_scale, x_max, y_offset_b - 50);
}

int draw_weather_grid()
{
	char label[32];
	struct tm timeinfo;
	time_t ts_start = meteo_graphs.graph_start;

	graph_array_t *g = &meteo_graphs.graphs[weatherIcon3h];

	if (g->len <= 0) {
		set_font_name(NULL);
		push_str(0, 0, "No\nweather\ndata", 17, A_CENTER, 600, 200);
		return -1;
	}

	// Draw 4 hours x 3 days weather forecast icons
	set_font_name(&f_weather_icons);
	for (int day_ind=0; day_ind<=2; day_ind++) {
		int y_pos = -((day_ind - 1) * 900);

		for (int h_ind=0; h_ind<=3; h_ind++) {
			int x_pos = (h_ind - 1) * 800 - 100;

			// Draw hour labels
			if (day_ind == 0) {
				snprintf(label, sizeof(label), "%2d h", h_ind * 2 * 3);
				set_font_name(NULL);
				push_str(x_pos, -1400, label, sizeof(label), A_CENTER, 300, 200);
			}

			int ind = day_ind * (24 / 3) + h_ind * 2;
			if (ind > g->len)
				return -2;

			int icon = g->data[ind] / 100;
			unsigned cp = cp_from_meteo_swiss_key(icon);

			set_font_name(&f_weather_icons);
			push_char_at_pos(x_pos, y_pos, cp, 500, 200);
		}

		// Draw weekday label
		localtime_r(&ts_start, &timeinfo);
		strftime(label, sizeof(label), "%a", &timeinfo);
		set_font_name(NULL);
		push_str(-1500, y_pos + 75, label, sizeof(label), A_CENTER, 300, 200);

		ts_start += 24 * 60 * 60;
	}
	return 0;
}

// dump the content of all arrays to stdout
void dump_graph(meteo_graph_t *mg)
{
	printf("graph_start: %d   graph_start_low: %d\n", mg->graph_start, mg->graph_start_low);
	for (unsigned i = 0; i < GRAPHS_N; i++) {
		graph_array_t *g = &mg->graphs[i];
		printf("%s  (%.1f, %.1f)", g->key, g->min / 100.0f, g->max / 100.0f);

		uint16_t len = g->len;
		int16_t *p = g->data;
		for (unsigned i=0; i<len; i++) {
			if ((i % 8) == 0)
				printf("\n");
			printf("%6.1f ", *p / 100.0f);
			p++;
		}
		printf("\n");
	}
}

// Parse the meteo-swiss PLZ API and store graph data in `meteo_graph_t` struct
static void json_cb_plz(lwjson_stream_parser_t* jsp, lwjson_stream_type_t type) {
	static graph_array_t *current_copy_dest = NULL;
	static unsigned n_copied = 0;
	static int16_t *weather_data_wp = NULL;
	static unsigned weather_data_written = 0;

	// printf("Got key '%s' with value '%s'\n", jsp->stack[sp - 1].meta.name, jsp->data.str.buff);
	// for (unsigned i=0; i < sp; i++)
	// 	printf("%10s ", lwjson_type_strings[jsp->stack[i].type]);
	// printf("\n");
	// return;

	int sp = jsp->stack_pos;

	if (sp == 1 && type == LWJSON_STREAM_TYPE_OBJECT) {
		weather_data_written = 0;
		weather_data_wp = meteo_graphs.weather_data;
		// printf("    START of .json\n");
		return;
	}

	if (sp == 0) {
		// printf("    END of .json\n");
		return;
	}

	// The graph header with timestamps
	if (sp == 4 && type == LWJSON_STREAM_TYPE_NUMBER && lwjson_stack_seq_4(jsp, 0,  OBJECT, KEY, OBJECT, KEY)) {
		if (strcmp(jsp->stack[sp - 1].meta.name, "start") == 0) {
			meteo_graphs.graph_start = atol(jsp->data.str.buff) / 1000;
			// printf("graph_start = %ld\n", meteo_graphs.graph_start);
			return;
		}
		if (strcmp(jsp->stack[sp - 1].meta.name, "startLowResolution") == 0) {
			meteo_graphs.graph_start_low = atol(jsp->data.str.buff) / 1000;
			// printf("graph_start_low = %ld\n", meteo_graphs.graph_start_low);
			return;
		}
	}

	// Little state machine to copy all the arrays into meteo_graphs.graphs
	if (current_copy_dest == NULL) {
		// Search for the beginning of the next graph array to copy
		if (!(
			sp == 5 &&
			type == LWJSON_STREAM_TYPE_ARRAY &&
			lwjson_stack_seq_4(jsp, 0,  OBJECT, KEY, OBJECT, KEY)
		))
			return;  // not an array

		// Check if the json-key matches any of the keys from meteo_graphs
		for (unsigned i = 0; i < GRAPHS_N; i++) {
			graph_array_t *g = &meteo_graphs.graphs[i];

			if (g->key == NULL || g->key[0] == '\0')
				return;

			if (strcmp(jsp->data.str.buff, g->key) != 0)
				continue;

			// The key does match, setup copy operation
			assert(weather_data_wp);
			current_copy_dest = g;
			current_copy_dest->data = weather_data_wp;
			current_copy_dest->len = 0;
			n_copied = 0;
			// printf("    COPY %s\n", current_copy_dest->key);
			return;
		}
	} else {
		if (type == LWJSON_STREAM_TYPE_ARRAY_END) {
			current_copy_dest->len = n_copied;
			// printf("    DONE %d items\n", current_copy_dest->len);

			current_copy_dest = NULL;
			return;
		}
		if (type == LWJSON_STREAM_TYPE_NUMBER) {
			if (weather_data_written >= WEATHER_DATA_N) {
				ESP_LOGE(T, "!!! weather_data overflow !!! (%s)\n", current_copy_dest->key);
				current_copy_dest->len = n_copied;
				current_copy_dest = NULL;
				return;
			}
			int16_t val = strtof(jsp->data.str.buff, NULL) * 100;

			if (n_copied == 0 || val < current_copy_dest->min)
				current_copy_dest->min = val;

			if (n_copied == 0 || val > current_copy_dest->max)
				current_copy_dest->max = val;

			*weather_data_wp++ = val;
			n_copied++;
			weather_data_written++;
		}
	}
}

// This is the esp_http_client event handler for streaming the GET response
// and decoding the .json on the fly (to minimize ram usage)
// "https://app-prod-ws.meteoswiss-app.ch/v1/plzDetail?plz=120200";
esp_err_t http_event_handler_plz(esp_http_client_event_t *evt)
{
	static lwjson_stream_parser_t stream_parser;

	switch(evt->event_id) {
		case HTTP_EVENT_ON_CONNECTED:
			ESP_LOGI(T, "HTTP_CONNECTED");
			lwjson_stream_init(&stream_parser, json_cb_plz);
			break;

		case HTTP_EVENT_ON_DATA: {
			int N = evt->data_len;
			char *p = evt->data;
			ESP_LOGD(T, "HTTP_DATA, len=%d", N);

			while (N--) {
				lwjsonr_t res = lwjson_stream_parse(&stream_parser, *p++);
				if (res == lwjsonSTREAMINPROG) {
				} else if (res == lwjsonSTREAMWAITFIRSTCHAR) {
					ESP_LOGI(T, "lwjson_stream_parse waiting for first character\n");
				} else if (res == lwjsonSTREAMDONE) {
					ESP_LOGI(T, "lwjson_stream_parse done\n");
					dump_graph(&meteo_graphs);
				} else {
					ESP_LOGE(T, "lwjson_stream_parse error: %d\n", res);
				}
			}
			break;

		default:
			break;
		}
	}
	return ESP_OK;
}

