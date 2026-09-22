#pragma once

#include <driver/pulse_cnt.h>

class PulseCounter {
    public:
        PulseCounter(uint8_t pin);
        ~PulseCounter();
        void start();
        void stop(bool clear = true);
        int getCount();
        void clearCount();
    private:
        uint8_t pin;
        pcnt_unit_handle_t unitHandle = nullptr;
        pcnt_channel_handle_t channelHandle = nullptr;
        void setupCounter();
};