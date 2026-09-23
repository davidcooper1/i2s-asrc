#include <pulse-counter.hpp>
#include <driver/pulse_cnt.h>
#include <esp_err.h>
#include <limits.h>

pcnt_unit_config_t unitConfig = {
    .group_id = 0,
    .clk_src = PCNT_CLK_SRC_DEFAULT,
    .low_limit = SHRT_MIN,
    .high_limit = SHRT_MAX,
    .intr_priority = 0,
    .flags {
        .accum_count = 1
    }
};

static bool onReachWatchPoint(pcnt_unit_handle_t unit, const pcnt_watch_event_data_t *data, void *userCtx) {
    return false;
}

pcnt_event_callbacks_t callbacks = {
    .on_reach = onReachWatchPoint
};

PulseCounter::PulseCounter(uint8_t pin, uint32_t glitchFilterNs) {
    ESP_ERROR_CHECK(pcnt_new_unit(&unitConfig, &unitHandle));

    if (glitchFilterNs != 0) {
        pcnt_glitch_filter_config_t glitchFilter = {
            .max_glitch_ns = glitchFilterNs
        };
        ESP_ERROR_CHECK(pcnt_unit_set_glitch_filter(unitHandle, &glitchFilter));
    }

    pcnt_chan_config_t config {
        .edge_gpio_num = pin,
        .level_gpio_num = -1,
        .flags {}
    };
    ESP_ERROR_CHECK(pcnt_new_channel(unitHandle, &config, &channelHandle));
    ESP_ERROR_CHECK(pcnt_channel_set_edge_action(channelHandle, PCNT_CHANNEL_EDGE_ACTION_INCREASE, PCNT_CHANNEL_EDGE_ACTION_HOLD));
    ESP_ERROR_CHECK(pcnt_unit_add_watch_point(unitHandle, unitConfig.high_limit));
    ESP_ERROR_CHECK(pcnt_unit_register_event_callbacks(unitHandle, &callbacks, nullptr));
    ESP_ERROR_CHECK(pcnt_unit_enable(unitHandle));
}

PulseCounter::~PulseCounter() {
    pcnt_del_unit(unitHandle);
    pcnt_del_channel(channelHandle);
}

void PulseCounter::start() {
    ESP_ERROR_CHECK(pcnt_unit_start(unitHandle));
}

void PulseCounter::stop(bool clear) {
    ESP_ERROR_CHECK(pcnt_unit_stop(unitHandle));
    if (clear) {
        clearCount();
    }
}

int PulseCounter::getCount() {
    int count;
    ESP_ERROR_CHECK(pcnt_unit_get_count(unitHandle, &count));
    return count;
}

void PulseCounter::clearCount() {
    ESP_ERROR_CHECK(pcnt_unit_clear_count(unitHandle));
}