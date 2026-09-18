#include <frequency-finder.hpp>
#include <esp_err.h>
#include <freertos/FreeRTOS.h>

namespace {
    uint32_t quantizeSampleRate(int sampleRate) {
        if (sampleRate >= 9512 && sampleRate < 13512) {
            return 11025;
        } else if (sampleRate >= 13512 && sampleRate < 19025) {
            return 16000;
        } else if (sampleRate >= 19025 && sampleRate < 27025) {
            return 22050;
        } else if (sampleRate >= 27025 && sampleRate < 38050) {
            return 32000;
        } else if (sampleRate >= 38050 && sampleRate < 46050) {
            return 44100;
        } else if (sampleRate >= 46050 && sampleRate < 69075) {
            return 48000;
        } else if (sampleRate >= 68100 && sampleRate < 92100) {
            return 88200;
        } else if (sampleRate >= 92100) {
            return 96000;
        }

        return 8000;
    };

    uint8_t quantizeBitsPerSample(int bits) {
        if (bits >= 12 && bits < 22) {
            return 16;
        } else if (bits >= 22 && bits < 26) {
            return 24;
        } else if (bits >= 26) {
            return 32;
        }
        
        return 8;
    };
}

FrequencyFinder::FrequencyFinder(gpio_num_t bckPin, gpio_num_t wsPin): bckCounter(bckPin), wsCounter(wsPin) {}

FrequencyFinder::~FrequencyFinder() {
    stopTask();
}

FrequencyInfo FrequencyFinder::getFrequencyInfo() {
    return info;
}

void FrequencyFinder::onFrequencyChange(FrequencyChangeCallback cb) {
    frequencyCallback = cb;
}

void FrequencyFinder::startTask() {
    if (frequencyCheckTaskHandle == nullptr) {
        xTaskCreate(
            &frequencyCheckTask,
            "FrequencyCheckTask",
            2048,
            this,
            5,
            &frequencyCheckTaskHandle
        );
    }
}

void FrequencyFinder::stopTask() {
    if (frequencyCheckTaskHandle != nullptr) {
        bckCounter.stop();
        wsCounter.stop();
        vTaskDelete(frequencyCheckTaskHandle);
        frequencyCheckTaskHandle = nullptr;
    }
}

void FrequencyFinder::frequencyCheckTask(void* params) {
    FrequencyFinder* instance = static_cast<FrequencyFinder*>(params);
    instance->bckCounter.start();
    instance->wsCounter.start();

    while(1) {
        vTaskDelay(pdMS_TO_TICKS(50));
        auto bckCount = instance->bckCounter.getCount() * 20;
        auto wsCount = instance->wsCounter.getCount() * 20;
        auto oldSampleRate = instance->info.sampleRate;
        auto oldBits = instance->info.bitsPerSample;

        instance->info.sampleRate = quantizeSampleRate(wsCount);
        instance->info.bitsPerSample = wsCount != 0 ? quantizeBitsPerSample(bckCount / wsCount / 2) : 0;

        if (instance->frequencyCallback != nullptr && (oldSampleRate != instance->info.sampleRate || oldBits != instance->info.bitsPerSample)) {
            instance->frequencyCallback();
        }

        instance->bckCounter.clearCount();
        instance->wsCounter.clearCount();
    }
}
