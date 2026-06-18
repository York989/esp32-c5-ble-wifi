file(GLOB_RECURSE srcs "*.c")

idf_component_register(
    SRCS "${srcs}"
    INCLUDE_DIRS "." "rgb_led"
    REQUIRES bsp espressif__led_strip esp_driver_gpio
)
