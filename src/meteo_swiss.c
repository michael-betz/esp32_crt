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
				return -1;

		if (i == 0 || y_val->valuedouble > max_val)
			max_val = y_val->valuedouble;

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

// Plot scaling
float rain_min = 0, rain_max = 0, temp_min = 0, temp_max = 0;

// meteo shall be the meteo-swiss .json data from
// https://app-prod-ws.meteoswiss-app.ch/v1/plzDetail?plz=xxx
// This needs to be locked against the drawing functions I guess
void weather_set_json(cJSON *meteo)
{
	if (weather != NULL)
		cJSON_Delete(weather);

	weather = meteo;

	// rescale plots
	get_min_max(&rain_min, NULL, "precipitationMin1h", 48);
	get_min_max(NULL, &rain_max, "precipitationMax1h", 48);
	printf("rain min/max: %.1f / %.1f mm\n", rain_min, rain_max);

	get_min_max(&temp_min, NULL, "temperatureMin1h", 48);
	get_min_max(NULL, &temp_max, "temperatureMax1h", 48);
	printf("temp min/max: %.1f / %.1f degC\n", temp_min, temp_max);
}

static int draw_plot(int y_offset, float dy, char *key, int max_points, float min_val, float max_val, int density)
{
	cJSON *array;
	int N = get_array_helper(key, &array, NULL);
	if (N < 0)
		return N;

	if (N > max_points)
		N = max_points;

	int i = 0;
	float x_val = 0;
	cJSON *y_val;

	float dx = (float)(PLOT_WIDTH) / N;

	cJSON_ArrayForEach(y_val, array) {
		if (!cJSON_IsNumber(y_val))
				return -1;

		int y = ((float)(y_val->valuedouble) - min_val) * dy + y_offset;

		if (i > N)
			break;

		x_val = (i - (float)(N) / 2) * dx + 200;

		if (i == 0)
			push_goto(x_val, y);
		else
			push_line(x_val, y, density);

		i++;
	}

	return 0;
}

static void draw_plot_x_axis(int y_offset, int N, int x_tick)
{
	char label[32];

	float dx = (float)(PLOT_WIDTH) / N;
	int n = N / x_tick;
	int x_offs = N * dx / 2;

	set_font_name(NULL);

	for (int i=0; i <= n; i++) {
		int x_val = i * x_tick * dx - x_offs + 200;
		push_goto(x_val, y_offset);
		push_line(x_val, y_offset - 75, 100);

		snprintf(label, sizeof(label), "%2dh", (i * x_tick) % 24);
		push_str(x_val, y_offset - 300, label, sizeof(label), A_CENTER, 200, 100);
	}
}

static void draw_plot_y_axis(int x_offset, int y_offset, float dy, float min_val, float max_val)
{
	char label[32];
	float ticks[] = {min_val, max_val};

	for (int i=0; i<2; i++) {
		float val = ticks[i];
		float y = y_offset + (val - min_val) * dy;

		push_goto(x_offset + 75, y);
		push_line(x_offset, y, 100);

		// Avoid overlapping digits in the axis ticks
		if (i > 0 && y < (y_offset + 180))
			y = y_offset + 180;

		set_font_name(NULL);
		snprintf(label, sizeof(label), "%.1f", val);

		push_str(x_offset, y, label, sizeof(label), A_RIGHT, 200, 100);
	}
}

void rain_temp_plot()
{
	int hours_to_plot = 48;
	draw_plot(300, 60, "precipitationMin1h", hours_to_plot, rain_min, rain_max, 50);
	draw_plot(300, 60, "precipitation1h", hours_to_plot, rain_min, rain_max, 200);
	draw_plot(300, 60, "precipitationMax1h", hours_to_plot, rain_min, rain_max, 50);
	draw_plot_y_axis(-1450, 300, 60, rain_min, rain_max);

	draw_plot(-1000, 70, "temperatureMin1h", hours_to_plot, temp_min, temp_max, 50);
	draw_plot(-1000, 70, "temperatureMean1h", hours_to_plot, temp_min, temp_max, 200);
	draw_plot(-1000, 70, "temperatureMax1h", hours_to_plot, temp_min, temp_max, 50);
	draw_plot_y_axis(-1450, -1000, 70, temp_min, temp_max);

	draw_plot_x_axis(-1000, hours_to_plot, 6);
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
