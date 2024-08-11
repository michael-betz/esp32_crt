#include <stdint.h>
#include <stdio.h>
#include <cjson/cJSON.h>
#include "meteo_swiss.h"
#include "fonts/font_data.h"

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
	cJSON *graph = cJSON_GetObjectItemCaseSensitive(weather, "graph");
	if (graph == NULL) {
		printf("graph not found\n");
		return -1;
	}

	// cJSON *start = cJSON_GetObjectItemCaseSensitive(graph, "start");
	// if (!cJSON_IsNumber(start)) {
	// 	printf("start not found\n");
	// 	return -1;
	// }
	// // start time is from midnight of the current day
	// printf("start: %ld\n", (long)(start->valuedouble));

	cJSON *icons = cJSON_GetObjectItemCaseSensitive(graph, "weatherIcon3h");
	if (icons == NULL) {
		printf("weatherIcon3h not found\n");
		return -1;
	}

	cJSON *icon = cJSON_GetArrayItem(icons, h);
	if (!cJSON_IsNumber(icon))
		return -1;

	unsigned cp = cp_from_meteo_swiss_key(icon->valueint);
	h *= 3;
	int d = h / 24;
	char label[32];
	if (d > 0)
		snprintf(label, sizeof(label), "+%d d %2d h", d, h % 24);
	else
		snprintf(label, sizeof(label), "%2d h", h);

	printf("%s  icon: %3d, %04x\n", label, icon->valueint, cp);

	set_font_name(&f_weather_icons);
	push_char_at_pos(0, -500, cp, 1800, 100);
	set_font_name(NULL);
	push_str(0, -1500, label, sizeof(label), A_CENTER, 400, 100);
}
