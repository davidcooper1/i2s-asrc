#pragma once
#include <pulse-counter.hpp>
#include <functional>
#include <driver/gpio.h>
#include <driver/pulse_cnt.h>
#include <freertos/FreeRTOS.h>

typedef struct {
    uint32_t sampleRate;
    uint8_t bitsPerSample;
} FrequencyInfo;

typedef std::function<void()> FrequencyChangeCallback;

class FrequencyFinder {
    public:
        /**
         * Creates an instance of frequency finder.
         */
        FrequencyFinder(gpio_num_t bckPin, gpio_num_t wsPin);
        ~FrequencyFinder();
        void onFrequencyChange(FrequencyChangeCallback cb);
        FrequencyInfo getFrequencyInfo();
        void startTask();
        void stopTask();

    private:
        static void frequencyCheckTask(void* params);
        TaskHandle_t frequencyCheckTaskHandle = nullptr;
        PulseCounter bckCounter;
        PulseCounter wsCounter;
        FrequencyChangeCallback frequencyCallback = nullptr;
        FrequencyInfo info;
};