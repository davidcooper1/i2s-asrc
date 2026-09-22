idf_component_register(
    SRCS "debounced-function.cpp" "ring-buffer.cpp" "pulse-counter.cpp" "i2s-asrc.cpp" "frequency-finder.cpp"
    INCLUDE_DIRS "./include"
    REQUIRES
        esp_driver_pcnt
        esp_driver_gpio
        freertos
)

idf_component_set_property(${COMPONENT_NAME} WHOLE_ARCHIVE ON)