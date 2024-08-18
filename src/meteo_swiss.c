#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include <time.h>
#include "meteo_swiss.h"
#include "draw.h"
#include "fonts/font_data.h"

#define PLOT_WIDTH 3000

cJSON *weather = NULL;

// see dev/2022-02-14-Wetter-Icons-inkl-beschreibung-v1-an-website.xlsx
static const uint8_t weather_icon_map_a[] = {
	0x0d,  // 1
	0x02,  // 2
	0x02,
	0x0c,
	0x13,
	0x08,
	0x06,
	0x65,
	0x08,
	0x0a,
	0x0a,
	0x05,
	0x05,
	0x1c,
	0x17,
	0x1b,
	0x19,
	0xb5,
	0x1b,
	0x19,
	0xb5,
	0x1b,
	0x1d,
	0x1e,
	0x10,
	0x7d,
	0x03,
	0x14,
	0x09,
	0x0a,
	0x09,
	0x09,
	0x07,
	0x65,
	0x41,
	0x6b,
	0x6b,
	0x0e,
	0x0e,
	0x05,
	0x05,
	0x6b  // 42
};

static const uint8_t weather_icon_map_b[] = {
	0x2e,  // 101
	0x81,  // 102
	0x86,
	0x31,
	0x13,
	0x29,
	0xb4,
	0x67,
	0x2b,
	0x2b,
	0x38,
	0x25,
	0x25,
	0x1c,
	0x17,
	0x1b,
	0x19,
	0xb5,
	0x1b,
	0x19,
	0xb5,
	0x1b,
	0x1d,
	0x1e,
	0x1e,
	0x80,
	0x4a,
	0x14,
	0x2b,
	0x2a,
	0x26,
	0x29,
	0x28,
	0x38,
	0x41,
	0x25,
	0x69,
	0x3a,
	0x6c,
	0x33,
	0x33,
	0x6c  // 142
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

static int get_array_helper(char *key, cJSON **array, time_t *ts_start)
{
	if (weather == NULL) {
		printf("weather data not available\n");
		return -1;
	}

	cJSON *graph = cJSON_GetObjectItemCaseSensitive(weather, "graph");
	if (graph == NULL) {
		printf("graph not found\n");
		return -1;
	}

	// extract POSIX timestamp in [s] from when the array data is valid
	if (ts_start != NULL) {
		cJSON *start_val = cJSON_GetObjectItemCaseSensitive(graph, "start");
		if (!cJSON_IsNumber(start_val)) {
			printf("start not found\n");
			return -1;
		}
		// start time is from midnight of the current day
		*ts_start = (long)(start_val->valuedouble) / 1000;
	}

	cJSON *array_ = cJSON_GetObjectItemCaseSensitive(graph, key);
	if (array_ == NULL) {
		printf("%s not found\n", key);
		return -1;
	}
	int N = cJSON_GetArraySize(array_);

	if (array != NULL) {
		*array = array_;
	}

	return N;
}

static int get_min_max(float *min, float *max, char *key, int max_points)
{
	int i = 0;
	float min_val = 0, max_val = 0;

	cJSON *y_val, *array;
	int N = get_array_helper(key, &array, NULL);
	if (N < 0)
		return N;

	cJSON_ArrayForEach(y_val, array) {
		if (!cJSON_IsNumber(y_val))
			break;

		if (max != NULL)
			if (i == 0 || y_val->valuedouble > max_val)
				max_val = y_val->valuedouble;

		if (min != NULL)
			if (i == 0 || y_val->valuedouble < min_val)
				min_val = y_val->valuedouble;

		if (i > max_points)
			break;

		i++;
	}

	if (min != NULL)
		*min = min_val;

	if (max != NULL)
		*max = max_val;

	return i;
}

// Minimum / Maximum values for plot scaling
float rain_min = 0, rain_max = 0, temp_min = 0, temp_max = 0;

// meteo shall be the meteo-swiss .json data from
// https://app-prod-ws.meteoswiss-app.ch/v1/plzDetail?plz=xxx
void weather_set_json(cJSON *meteo)
{
	if (weather != NULL)
		cJSON_Delete(weather);

	weather = meteo;

	// get min / max values to get the plot scaling right
	float tmp_a, tmp_b, tmp_c, tmp_d;

	get_min_max(&tmp_a, NULL, "precipitationMin10m", 100);
	get_min_max(&tmp_b, NULL, "precipitationMin1h", 43);
	rain_min = tmp_a < tmp_b ? tmp_a : tmp_b;

	get_min_max(NULL, &tmp_c, "precipitationMax10m", 100);
	get_min_max(NULL, &tmp_d, "precipitationMax1h", 43);
	rain_max = tmp_c > tmp_d ? tmp_c : tmp_d;
	printf("rain min/max: %.1f / %.1f mm\n", rain_min, rain_max);

	get_min_max(&temp_min, NULL, "temperatureMin1h", 58);
	get_min_max(NULL, &temp_max, "temperatureMax1h", 58);
	printf("temp min/max: %.1f / %.1f degC\n", temp_min, temp_max);
}

static int draw_plot(float x_offset, float dx, float max_x, int y_offset, float dy, char *key, float min_val, int density)
{
	cJSON *array;
	int N = get_array_helper(key, &array, NULL);
	if (N < 0)
		return x_offset;

	float x = x_offset;
	cJSON *array_val;
	// int i = 0;

	cJSON_ArrayForEach(array_val, array) {
		if (x > max_x)
			break;

		if (!cJSON_IsNumber(array_val))
				break;

		int y = ((float)(array_val->valuedouble) - min_val) * dy + y_offset;

		if (x == x_offset)
			push_goto(x, y);
		else
			push_line(x, y, density);

		x += dx;
		// i++;
	}
	// printf("%s plotted %d points\n", key, i);

	return x;
}

static void draw_plot_x_axis(int x_offset, float dx, float dHours, float max_x, int y_offset)
{
	char label[32];
	set_font_name(NULL);

	float hours = 0;

	push_str(x_offset + dx / 2, y_offset - 300, "[h]", 3, A_CENTER, 150, 100);

	for (float x = x_offset; x <= max_x; x += dx) {
		push_goto(x, y_offset);
		push_line(x, y_offset - 75, 100);

		snprintf(label, sizeof(label), "%2d", ((int)(hours + 0.5)) % 24);
		push_str(x, y_offset - 300, label, sizeof(label), A_CENTER, 200, 100);
		hours += dHours;
	}
}

static void draw_plot_y_axis(int x_offset, int y_offset, float dy, float min_val, float max_val)
{
	char label[32];
	float ticks[] = {min_val, max_val};

	for (int i=0; i<2; i++) {
		float val = ticks[i];
		float y = y_offset + (val - min_val) * dy;

		push_goto(x_offset - 75, y);
		push_line(x_offset, y, 100);

		// Avoid overlapping digits in the axis ticks
		if (i > 0 && y < (y_offset + 180))
			y = y_offset + 180;

		set_font_name(NULL);
		snprintf(label, sizeof(label), "%.1f", val);

		push_str(x_offset - 75, y, label, sizeof(label), A_RIGHT, 200, 100);
	}
}

void rain_temp_plot(unsigned zoom)
{
	int x_end = 0;

	zoom++;
	if (zoom > 6)
		zoom = 6;

	const float x_scale = 5.0 * zoom; // 10.0 pixels per sample, 10 minutes / pixel
	const float y_scale = 60.0;
	const float x_min = -1500.0;  // pixels
	const float x_max = 1900.0;  // pixels

	// Rain plot
	const int y_offset_a = 400;
	push_str(0, y_offset_a + 1000, "[mm/h]", 6, A_CENTER, 250, 150);
	draw_plot_y_axis(x_min - 25.0, y_offset_a, y_scale, rain_min, rain_max);

	x_end = draw_plot(x_min, x_scale, x_max, y_offset_a, y_scale, "precipitation10m", rain_min, 200);
	draw_plot(x_end, x_scale * 6.0, x_max, y_offset_a, y_scale, "precipitation1h", rain_min, 200);

	x_end = draw_plot(x_min, x_scale, x_max, y_offset_a, y_scale, "precipitationMax10m", rain_min, 50);
	draw_plot(x_end, x_scale * 6.0, x_max, y_offset_a, y_scale, "precipitationMax1h", rain_min, 50);

	// Temperature plot
	const int y_offset_b = -1100;
	push_str(0, y_offset_b + 1000, "[°C]", 5, A_CENTER, 250, 150);
	draw_plot_y_axis(x_min - 25.0, y_offset_b, y_scale, temp_min, temp_max);
	draw_plot(x_min, x_scale * 6.0, x_max, y_offset_b, y_scale, "temperatureMax1h", temp_min, 50);
	draw_plot(x_min, x_scale * 6.0, x_max, y_offset_b, y_scale, "temperatureMean1h", temp_min, 200);
	draw_plot(x_min, x_scale * 6.0, x_max, y_offset_b, y_scale, "temperatureMin1h", temp_min, 50);

	// x-axis: Draw a tick every 6 h
	draw_plot_x_axis(x_min, x_scale * 6.0 * 12.0 / zoom, 12.0 / zoom, x_max, y_offset_b - 25);
}

int draw_weather_grid()
{
	char label[32];
	cJSON *icons;
	time_t ts_start;
	struct tm timeinfo;

	int N = get_array_helper("weatherIcon3h", &icons, &ts_start);
	if (N < 0)
		return N;

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
				push_str(x_pos, -1400, label, sizeof(label), A_CENTER, 300, 75);
			}

			int ind = day_ind * (24 / 3) + h_ind * 2;
			cJSON *icon = cJSON_GetArrayItem(icons, ind);
			if (!cJSON_IsNumber(icon))
				return -1;

			set_font_name(&f_weather_icons);
			unsigned cp = cp_from_meteo_swiss_key(icon->valueint);
			push_char_at_pos(x_pos, y_pos, cp, 500, 75);
		}

		// Draw weekday label
		localtime_r(&ts_start, &timeinfo);
    	strftime(label, sizeof(label), "%a", &timeinfo);
		set_font_name(NULL);
		push_str(-1500, y_pos + 75, label, sizeof(label), A_CENTER, 300, 75);

    	ts_start += 24 * 60 * 60;
	}
	return 0;
}
