#pragma once

#if defined(ESP_PLATFORM)
    #include <cJSON.h>
#else
    #include <cjson/cJSON.h>
#endif

void weather_set_json(cJSON *meteo);

int draw_weather_grid();

int draw_plot(int dx, int dy, char *key, int max_x, bool do_axes, bool is_limit);


