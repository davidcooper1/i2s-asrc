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

#define BIT_IN_BCLK 1 << 0
#define BIT_IN_WS 1 << 1
#define BIT_IN_DIN 1 << 2
#define BIT_OUT_BCLK 1 << 3
#define BIT_OUT_WS 1 << 4

#define BIT_OUT_DOUT 1 << 0

#define INPUT_BIT_MASK (BIT_IN_BCLK | BIT_IN_WS | BIT_IN_DIN | BIT_OUT_BCLK | BIT_OUT_WS)
#define OUTPUT_BIT_MASK BIT_OUT_DOUT

#if !defined(CONFIG_SOC_DEDICATED_GPIO_SUPPORTED)
    #error "Dedicated GPIO must be supported and enabled."
#endif

// Core that will run bit-bang loop.
#define BIT_BANG_CORE_ID 1

namespace {
    struct AudioFrame {
        int32_t left;
        int32_t right;
    };

    RingBuffer<AudioFrame, 256> in;
    RingBuffer<AudioFrame, 256> out;

    dedic_gpio_bundle_handle_t gpioInHandle = nullptr;
    dedic_gpio_bundle_handle_t gpioOutHandle = nullptr;

    // Uses 
    inline void setDataBit(bool enabled) {
        if (enabled) {
            asm volatile ("ee.set_bit_gpio_out %0" : : "I"(BIT_OUT_DOUT) :);
        } else {
            asm volatile ("ee.clr_bit_gpio_out %0" : : "I"(BIT_OUT_DOUT) :);
        }
    }

    inline uint32_t getInputBits() {
        uint32_t result;
        asm volatile ("ee.get_gpio_in %[r]" : [r] "=r" (result));
        return result;
    }

    /**
     * Configures the GPIO pins and initializes the dedicated GPIO module.
     * @param taskHandle The pointer to task waiting on this one.
     */
    void GpioInitializeTask(void* taskHandle) {
        TaskHandle_t waitingTaskHandle = static_cast<TaskHandle_t>(taskHandle);

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

        dedic_gpio_bundle_config_t bundleConfig = {
            .gpio_array = new const int[]{ 
                IN_BCLK_PIN, IN_WS_PIN, IN_DIN_PIN,
                OUT_BCLK_PIN, OUT_WS_PIN
            },
            .array_size = 5,
            .flags = {
                .in_en = 1,
                .in_invert = 0,
                .out_en = 0,
                .out_invert = 0
            }
        };
        ESP_ERROR_CHECK(dedic_gpio_new_bundle(&bundleConfig, &gpioInHandle));

        bundleConfig = {
            .gpio_array = new const int[] { OUT_DOUT_PIN },
            .array_size = 1,
            .flags = {
                .in_en = 0,
                .in_invert = 0,
                .out_en = 1,
                .out_invert = 0
            }
        };
        ESP_ERROR_CHECK(dedic_gpio_new_bundle(&bundleConfig, &gpioOutHandle));

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