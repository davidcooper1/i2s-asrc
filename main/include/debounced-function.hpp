#pragma once
#include <functional>
#include <freertos/FreeRTOS.h>

class DebouncedFunction {
    public:
        DebouncedFunction(std::function<void()> cb, unsigned int delayMs);
        ~DebouncedFunction();

        void operator()();
    private:
        TimerHandle_t timerHandle;
        std::function<void()> callback = nullptr;

        static void doCallback(TimerHandle_t timer);
};
