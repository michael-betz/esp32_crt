#pragma once

#include "esp_http_client.h"

// void weather_set_json(cJSON *meteo);

int draw_weather_grid();

void rain_temp_plot(unsigned zoom);

esp_err_t http_event_handler_plz(esp_http_client_event_t *evt);
