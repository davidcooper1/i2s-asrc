#pragma once

#include <driver/pulse_cnt.h>

class PulseCounter {
    public:
        PulseCounter(uint8_t pin, uint32_t glitchFilterNs = 0);
        ~PulseCounter();
        void start();
        void stop(bool clear = true);
        int getCount();
        void clearCount();
    private:
        pcnt_unit_handle_t unitHandle = nullptr;
        pcnt_channel_handle_t channelHandle = nullptr;
};