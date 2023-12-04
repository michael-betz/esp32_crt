#ifndef STATIC_WS_H
#define STATIC_WS_H
// ESP_IDF is too verbose
#define E(x) ESP_ERROR_CHECK_WITHOUT_ABORT(x)
#define log_e(format, ...) ESP_LOGE(T, format, ##__VA_ARGS__)
#define log_w(format, ...) ESP_LOGW(T, format, ##__VA_ARGS__)
#define log_i(format, ...) ESP_LOGI(T, format, ##__VA_ARGS__)
#define log_d(format, ...) ESP_LOGD(T, format, ##__VA_ARGS__)
#define log_v(format, ...) ESP_LOGV(T, format, ##__VA_ARGS__)

#define MIN(a, b) ((a) < (b) ? (a) : (b))
#define MAX(a, b) ((a) > (b) ? (a) : (b))

void startWebServer();
void stopWebServer();

#endif
