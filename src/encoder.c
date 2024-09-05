#include "esp_log.h"
#include "driver/pulse_cnt.h"
#include "driver/gpio.h"
#include "main.h"

static const char *TAG = "ENC";

static pcnt_unit_handle_t pcnt_unit = NULL;

void init_encoder(void)
{
    ESP_LOGI(TAG, "install pcnt unit");
    pcnt_unit_config_t unit_config = {
        .high_limit = 32767,
        .low_limit = -32767,
    };
    ESP_ERROR_CHECK(pcnt_new_unit(&unit_config, &pcnt_unit));

    ESP_LOGI(TAG, "set glitch filter");
    pcnt_glitch_filter_config_t filter_config = {
        .max_glitch_ns = 1000,
    };
    ESP_ERROR_CHECK(pcnt_unit_set_glitch_filter(pcnt_unit, &filter_config));

    ESP_LOGI(TAG, "install pcnt channels");
    pcnt_chan_config_t chan_a_config = {
        .edge_gpio_num = PIN_KNOB_A,
        .level_gpio_num = PIN_KNOB_B,
    };
    pcnt_channel_handle_t pcnt_chan_a = NULL;
    ESP_ERROR_CHECK(pcnt_new_channel(pcnt_unit, &chan_a_config, &pcnt_chan_a));
    pcnt_chan_config_t chan_b_config = {
        .edge_gpio_num = PIN_KNOB_B,
        .level_gpio_num = PIN_KNOB_A,
    };
    pcnt_channel_handle_t pcnt_chan_b = NULL;
    ESP_ERROR_CHECK(pcnt_new_channel(pcnt_unit, &chan_b_config, &pcnt_chan_b));

    ESP_LOGI(TAG, "set edge and level actions for pcnt channels");
    ESP_ERROR_CHECK(pcnt_channel_set_edge_action(pcnt_chan_a, PCNT_CHANNEL_EDGE_ACTION_DECREASE, PCNT_CHANNEL_EDGE_ACTION_INCREASE));
    ESP_ERROR_CHECK(pcnt_channel_set_level_action(pcnt_chan_a, PCNT_CHANNEL_LEVEL_ACTION_KEEP, PCNT_CHANNEL_LEVEL_ACTION_INVERSE));
    ESP_ERROR_CHECK(pcnt_channel_set_edge_action(pcnt_chan_b, PCNT_CHANNEL_EDGE_ACTION_INCREASE, PCNT_CHANNEL_EDGE_ACTION_DECREASE));
    ESP_ERROR_CHECK(pcnt_channel_set_level_action(pcnt_chan_b, PCNT_CHANNEL_LEVEL_ACTION_KEEP, PCNT_CHANNEL_LEVEL_ACTION_INVERSE));

    ESP_LOGI(TAG, "enable pcnt unit");
    ESP_ERROR_CHECK(pcnt_unit_enable(pcnt_unit));
    ESP_LOGI(TAG, "clear pcnt unit");
    ESP_ERROR_CHECK(pcnt_unit_clear_count(pcnt_unit));
    ESP_LOGI(TAG, "start pcnt unit");
    ESP_ERROR_CHECK(pcnt_unit_start(pcnt_unit));
}

void clear_encoder()
{
    ESP_ERROR_CHECK(pcnt_unit_clear_count(pcnt_unit));
}

int8_t get_encoder(unsigned *state)
{
    static int pulse_count_ = 0;
    static unsigned btn_state_ = 0;
    int pulse_count = 0;
    ESP_ERROR_CHECK(pcnt_unit_get_count(pcnt_unit, &pulse_count));

    pulse_count >>= 2;

    int8_t enc_diff = pulse_count - pulse_count_;
    pulse_count_ = pulse_count;

    if (state != NULL) {
        unsigned btn_state = gpio_get_level(PIN_BUTTON);
        unsigned rising = (~btn_state_) & btn_state;
        unsigned falling = btn_state_ & (~btn_state);
        *state = (enc_diff << 24) | (falling << 16) | (rising << 8) | btn_state;
        btn_state_ = btn_state;
    }

    return enc_diff;
}
