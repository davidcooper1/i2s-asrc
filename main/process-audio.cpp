#include <process-audio.hpp>
#include <stdint.h>
#include <ring-buffer.hpp>
#include <esp_err.h>
#include <driver/gpio.h>
#include <driver/dedic_gpio.h>
#include <freertos/FreeRTOS.h>

// Input side BCLK input pin.
#define IN_BCLK_PIN GPIO_NUM_9
// Input side WS input pin.
#define IN_WS_PIN GPIO_NUM_11
// Input side DIN input pin.
#define IN_DIN_PIN GPIO_NUM_10

// Output side BCLK input pin.
#define OUT_BCLK_PIN GPIO_NUM_5
// Output side WS input pin.
#define OUT_WS_PIN GPIO_NUM_6
// Output side DOUT output pin.
#define OUT_DOUT_PIN GPIO_NUM_4

// Mask of all input GPIO pins.
#define INPUT_PIN_MASK (\
    (1 << IN_BCLK_PIN)\
    | (1 << IN_WS_PIN)\
    | (1 << IN_DIN_PIN)\
    | (1 << OUT_BCLK_PIN)\
    | (1 << OUT_WS_PIN)\
)

// Mask of all output GPIO pins.
#define OUTPUT_PIN_MASK ((1 << OUT_DOUT_PIN))

// Core that will run bit-bang loop.
#define BIT_BANG_CORE_ID 1

namespace {
    struct AudioFrame {
        int32_t left;
        int32_t right;
    };

    RingBuffer<AudioFrame, 256> in;

    dedic_gpio_bundle_handle_t gpioHandle = nullptr;

    void GpioInitializeTask(void* params) {
        TaskHandle_t waitingTaskHandle = static_cast<TaskHandle_t>(params);

        gpio_config_t config = {
            .pin_bit_mask = INPUT_PIN_MASK,
            .mode = GPIO_MODE_INPUT,
            .pull_up_en = GPIO_PULLUP_DISABLE,
            .pull_down_en = GPIO_PULLDOWN_ENABLE,
            .intr_type = GPIO_INTR_DISABLE
        };
        ESP_ERROR_CHECK(gpio_config(&config));

        config.pin_bit_mask = OUTPUT_PIN_MASK;
        config.mode = GPIO_MODE_OUTPUT;
        ESP_ERROR_CHECK(gpio_config(&config));

        if (waitingTaskHandle != nullptr) {
            xTaskNotifyGive(waitingTaskHandle);
        }

        vTaskDelete(nullptr);
    }
}

// Initializes GPIO using bit-bang core and waits until completed.
void initializeGpio() {
    TaskHandle_t currentTaskHandle = xTaskGetCurrentTaskHandle();
    TaskHandle_t gpioInitTaskHandle = nullptr;
    BaseType_t status = xTaskCreatePinnedToCore(
        GpioInitializeTask,
        "Initialize GPIO",
        1024,
        static_cast<void*>(currentTaskHandle),
        1,
        &gpioInitTaskHandle,
        BIT_BANG_CORE_ID
    );

    assert(status == pdPASS && "Initialization of GPIO failed.");
    
    ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
}