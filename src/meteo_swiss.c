#include <stdint.h>
#include <stdio.h>
#include <time.h>
#include <cjson/cJSON.h>
#include "meteo_swiss.h"
#include "fonts/font_data.h"

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

int draw_weather_icons(cJSON *weather, unsigned h)
{
	char label[32];

	cJSON *graph = cJSON_GetObjectItemCaseSensitive(weather, "graph");
	if (graph == NULL) {
		printf("graph not found\n");
		return -1;
	}

	cJSON *start = cJSON_GetObjectItemCaseSensitive(graph, "start");
	if (!cJSON_IsNumber(start)) {
		printf("start not found\n");
		return -1;
	}
	// start time is from midnight of the current day
	time_t ts_start;
	struct tm timeinfo;
	// time(&ts_start);
	// printf("time: %ld\n", ts_start);
	ts_start = (long)(start->valuedouble) / 1000;
	printf("start: %ld\n", ts_start);

	cJSON *icons = cJSON_GetObjectItemCaseSensitive(graph, "weatherIcon3h");
	if (icons == NULL) {
		printf("weatherIcon3h not found\n");
		return -1;
	}

	// Draw 3 times x 3 hours of weather icons
	set_font_name(&f_weather_icons);
	for (int day_ind=0; day_ind<=2; day_ind++) {
		int x_pos = (day_ind - 1) * 750 + 300;

		for (int h_ind=0; h_ind<=3; h_ind++) {
			int y_pos = -((h_ind - 2) * 750 + 300);

			// Draw hour labels
			if (day_ind == 0) {
				snprintf(label, sizeof(label), "%2d h", h_ind * 2 * 3);
				set_font_name(NULL);
				push_str(-1100, y_pos + 100, label, sizeof(label), A_CENTER, 300, 100);
			}

			int ind = day_ind * (24 / 3) + h_ind * 2;
			cJSON *icon = cJSON_GetArrayItem(icons, ind);
			if (!cJSON_IsNumber(icon))
				return -1;

			set_font_name(&f_weather_icons);
			unsigned cp = cp_from_meteo_swiss_key(icon->valueint);
			push_char_at_pos(x_pos, y_pos, cp, 500, 100);
		}

		// Draw weekday label
		localtime_r(&ts_start, &timeinfo);
    	strftime(label, sizeof(label), "%a", &timeinfo);
		set_font_name(NULL);
		push_str(x_pos, -1500, label, sizeof(label), A_CENTER, 300, 100);

    	ts_start += 24 * 60 * 60;
	}

	// h *= 3;
	// int d = h / 24;
	// if (d > 0)
	// 	snprintf(label, sizeof(label), "+%d d %2d h", d, h % 24);
	// else
	// 	snprintf(label, sizeof(label), "%2d h", h);

	// printf("%s  icon: %3d, %04x\n", label, icon->valueint, cp);

	// set_font_name(&f_weather_icons);
	// push_char_at_pos(0, -500, cp, 1800, 100);
	// push_str(0, -1500, label, sizeof(label), A_CENTER, 400, 100);
}
