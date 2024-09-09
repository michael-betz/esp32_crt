#pragma once

void request_weather_data();

// Shows 4 x 3 weather forecast symbols
int draw_weather_grid(int par);

// Shows a single big weater forecast symbol
int draw_weather_symbol(int par);

int rain_temp_plot(int x_scale);
