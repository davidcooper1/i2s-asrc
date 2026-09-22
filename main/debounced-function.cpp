#include <debounced-function.hpp>

DebouncedFunction::DebouncedFunction(std::function<void()> cb, unsigned int delayMs) {
    callback = cb;
    timerHandle = xTimerCreate(
        "Debounce Timer",
        pdMS_TO_TICKS(delayMs),
        pdFALSE,
        (void*)this,
        &doCallback
    );
}

DebouncedFunction::~DebouncedFunction() {
    xTimerDelete(timerHandle, 0);
}

void DebouncedFunction::operator()() {
    xTimerStart(timerHandle, 0);
}

void DebouncedFunction::doCallback(TimerHandle_t timer) {
    DebouncedFunction* instance = (DebouncedFunction*)pvTimerGetTimerID(timer);
    if (instance->callback != nullptr) {
        instance->callback();
    }
}