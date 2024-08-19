#pragma once

// #if defined(ESP_PLATFORM)
    #include <cJSON.h>
// #else
//     #include <cjson/cJSON.h>
// #endif

void weather_set_json(cJSON *meteo);

int draw_weather_grid();

void rain_temp_plot();
