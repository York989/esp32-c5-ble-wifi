file(GLOB_RECURSE srcs "*.c")

idf_component_register(
    SRCS "${srcs}"
    INCLUDE_DIRS "." "ble" "heart_rate" "led_effect"
    PRIV_REQUIRES bt drv esp_hw_support nvs_flash
)
