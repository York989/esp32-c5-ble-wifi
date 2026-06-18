/*
 * SPDX-FileCopyrightText: 2024 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: Unlicense OR CC0-1.0
 */
/* Includes */
#include "drv_rgb_led.h"

#include <stdbool.h>

#include "bsp_board.h"
#include "esp_log.h"
#include "sdkconfig.h"

#ifdef CONFIG_BLINK_LED_GPIO
#include "driver/gpio.h"
#endif

#ifdef CONFIG_BLINK_LED_STRIP
#include "led_strip.h"
#endif

/* Private variables */
static const char *const s_drv_rgb_led_log_tag = "drv_rgb_led";
static bool s_drv_rgb_led_is_initialized;

#ifdef CONFIG_BLINK_LED_STRIP
static led_strip_handle_t s_drv_rgb_led_strip;
#endif

/* Private functions */
#ifdef CONFIG_BLINK_LED_STRIP
static esp_err_t drv_rgb_led_write_strip(uint8_t red, uint8_t green,
                                         uint8_t blue) {
    esp_err_t err;

    err = led_strip_set_pixel(s_drv_rgb_led_strip, 0, red, green, blue);
    if (err != ESP_OK) {
        return err;
    }

    return led_strip_refresh(s_drv_rgb_led_strip);
}
#endif

/* Public functions */
esp_err_t drv_rgb_led_init(void) {
    esp_err_t err;

    if (s_drv_rgb_led_is_initialized) {
        return ESP_OK;
    }

#ifdef CONFIG_BLINK_LED_STRIP
    ESP_LOGI(s_drv_rgb_led_log_tag, "initializing WS2812 RGB LED on GPIO%d",
             g_bsp_board_info.rgb_led_gpio);

    led_strip_config_t strip_config = {
        .strip_gpio_num = g_bsp_board_info.rgb_led_gpio,
        .max_leds = 1,
        .led_model = LED_MODEL_WS2812,
        .color_component_format = LED_STRIP_COLOR_COMPONENT_FMT_GRB,
    };

#if CONFIG_BLINK_LED_STRIP_BACKEND_RMT
    led_strip_rmt_config_t rmt_config = {
        .resolution_hz = 10 * 1000 * 1000,
        .flags.with_dma = false,
    };
    err = led_strip_new_rmt_device(&strip_config, &rmt_config,
                                   &s_drv_rgb_led_strip);
#elif CONFIG_BLINK_LED_STRIP_BACKEND_SPI
    led_strip_spi_config_t spi_config = {
        .spi_bus = SPI2_HOST,
        .flags.with_dma = true,
    };
    err = led_strip_new_spi_device(&strip_config, &spi_config,
                                   &s_drv_rgb_led_strip);
#else
#error "unsupported LED strip backend"
#endif

    if (err != ESP_OK) {
        ESP_LOGE(s_drv_rgb_led_log_tag,
                 "failed to initialize RGB LED strip GPIO%d: %s (%d)",
                 g_bsp_board_info.rgb_led_gpio, esp_err_to_name(err), err);
        return err;
    }
#elif CONFIG_BLINK_LED_GPIO
    ESP_LOGI(s_drv_rgb_led_log_tag, "initializing GPIO LED on GPIO%d",
             g_bsp_board_info.rgb_led_gpio);

    gpio_reset_pin(g_bsp_board_info.rgb_led_gpio);
    err = gpio_set_direction(g_bsp_board_info.rgb_led_gpio, GPIO_MODE_OUTPUT);
    if (err != ESP_OK) {
        ESP_LOGE(s_drv_rgb_led_log_tag,
                 "failed to configure RGB LED GPIO%d: %s (%d)",
                 g_bsp_board_info.rgb_led_gpio, esp_err_to_name(err), err);
        return err;
    }
#else
#error "unsupported LED type"
#endif

    s_drv_rgb_led_is_initialized = true;
    return drv_rgb_led_clear();
}

esp_err_t drv_rgb_led_set_rgb(uint8_t red, uint8_t green, uint8_t blue) {
    if (!s_drv_rgb_led_is_initialized) {
        return ESP_ERR_INVALID_STATE;
    }

#ifdef CONFIG_BLINK_LED_STRIP
    return drv_rgb_led_write_strip(red, green, blue);
#elif CONFIG_BLINK_LED_GPIO
    return gpio_set_level(g_bsp_board_info.rgb_led_gpio,
                          (red || green || blue) ? 1 : 0);
#else
#error "unsupported LED type"
#endif
}

esp_err_t drv_rgb_led_clear(void) {
    if (!s_drv_rgb_led_is_initialized) {
        return ESP_ERR_INVALID_STATE;
    }

#ifdef CONFIG_BLINK_LED_STRIP
    return led_strip_clear(s_drv_rgb_led_strip);
#elif CONFIG_BLINK_LED_GPIO
    return gpio_set_level(g_bsp_board_info.rgb_led_gpio, 0);
#else
#error "unsupported LED type"
#endif
}
