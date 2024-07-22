#ifndef WIFI_H
#define WIFI_H

#define WIFI_HOST_NAME "esp_crt"
extern bool isConnect;

void initWifi();
void tryJsonConnect();
void tryEasyConnect();

#endif
