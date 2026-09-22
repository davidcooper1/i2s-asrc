#include <stdio.h>
#include <driver/gpio.h>
#include <frequency-finder.hpp>
#include <debounced-function.hpp>

FrequencyFinder in(GPIO_NUM_9, GPIO_NUM_11);
FrequencyFinder out(GPIO_NUM_5, GPIO_NUM_6);

TaskHandle_t I2SLoopTask = nullptr;

void I2SLoop(void* params) {
    while(1) {
        printf("In loop\n");
        vTaskDelay(pdMS_TO_TICKS(2000));
    }
}

void startTask() {
    printf("Task restarted.\n");
    if (I2SLoopTask == nullptr) {
        auto inInfo = in.getFrequencyInfo();
        auto outInfo = out.getFrequencyInfo();

        printf("[In] Sample: %lu Bits: %u\n", inInfo.sampleRate, inInfo.bitsPerSample);
        printf("[Out] Sample: %lu Bits: %u\n", outInfo.sampleRate, outInfo.bitsPerSample);

        xTaskCreatePinnedToCore(
            I2SLoop,
            "I2S Loop",
            4096,
            nullptr,
            10,
            &I2SLoopTask,
            1
        );
    }
}

DebouncedFunction restart(&startTask, 1000);

void queueRestart() {
    if (I2SLoopTask != nullptr) {
        vTaskDelete(I2SLoopTask);
        I2SLoopTask = nullptr;
    }

    restart();
}

extern "C" void app_main(void) {
    in.onFrequencyChange(&queueRestart);
    out.onFrequencyChange(&queueRestart);
    in.startTask();
    out.startTask();
}
    

