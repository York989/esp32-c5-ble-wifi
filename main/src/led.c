/*
 * SPDX-FileCopyrightText: 2024 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: Unlicense OR CC0-1.0
 */
/* Includes */
#include "led.h"
#include "common.h"

/* Private variables */
static uint8_t led_state;
static volatile bool rainbow_enabled = true;

#ifdef CONFIG_BLINK_LED_STRIP
static led_strip_handle_t led_strip;
#if CONFIG_BLINK_LED_STRIP_BACKEND_RMT
static led_strip_handle_t led_probe_strip;
static int led_probe_gpio = -1;
#endif
#endif

/* Public functions */
uint8_t get_led_state(void) { return led_state; }

bool led_is_rainbow_enabled(void) { return rainbow_enabled; }

#ifdef CONFIG_BLINK_LED_STRIP

static esp_err_t led_write_strip(led_strip_handle_t strip, uint8_t red,
                                 uint8_t green, uint8_t blue) {
    /* Set the LED pixel using RGB from 0 (0%) to 255 (100%) for each color */
    esp_err_t ret = led_strip_set_pixel(strip, 0, red, green, blue);
    if (ret != ESP_OK) {
        return ret;
    }

    /* Refresh the strip to send data */
    return led_strip_refresh(strip);
}

esp_err_t led_set_rgb(uint8_t red, uint8_t green, uint8_t blue) {
    esp_err_t ret = led_write_strip(led_strip, red, green, blue);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "failed to update main RGB LED GPIO%d: %s (%d)",
                 CONFIG_BLINK_GPIO, esp_err_to_name(ret), ret);
        return ret;
    }

#if CONFIG_BLINK_LED_STRIP_BACKEND_RMT
    if (led_probe_strip) {
        ret = led_write_strip(led_probe_strip, red, green, blue);
        if (ret != ESP_OK) {
            ESP_LOGE(TAG, "failed to update RGB probe GPIO%d: %s (%d)",
                     led_probe_gpio, esp_err_to_name(ret), ret);
            return ret;
        }
    }
#endif

    /* Update LED state */
    led_state = (red || green || blue);
    return ESP_OK;
}

esp_err_t led_set_probe_gpio(int gpio_num) {
#if CONFIG_BLINK_LED_STRIP_BACKEND_RMT
    esp_err_t ret;

    if (gpio_num == CONFIG_BLINK_GPIO) {
        gpio_num = -1;
    }

    if (gpio_num == led_probe_gpio) {
        return ESP_OK;
    }

    if (led_probe_strip) {
        led_strip_clear(led_probe_strip);
        ret = led_strip_del(led_probe_strip);
        if (ret != ESP_OK) {
            ESP_LOGE(TAG, "failed to delete RGB probe GPIO%d: %s (%d)",
                     led_probe_gpio, esp_err_to_name(ret), ret);
            return ret;
        }
        led_probe_strip = NULL;
        led_probe_gpio = -1;
    }

    if (gpio_num < 0) {
        return ESP_OK;
    }

    led_strip_config_t strip_config = {
        .strip_gpio_num = gpio_num,
        .max_leds = 1,
        .led_model = LED_MODEL_WS2812,
        .color_component_format = LED_STRIP_COLOR_COMPONENT_FMT_GRB,
    };
    led_strip_rmt_config_t rmt_config = {
        .resolution_hz = 10 * 1000 * 1000,
        .flags.with_dma = false,
    };

    ret = led_strip_new_rmt_device(&strip_config, &rmt_config,
                                   &led_probe_strip);
    if (ret != ESP_OK) {
        ESP_LOGW(TAG, "failed to enable RGB probe on GPIO%d: %s (%d)",
                 gpio_num, esp_err_to_name(ret), ret);
        return ret;
    }

    led_probe_gpio = gpio_num;
    led_strip_clear(led_probe_strip);
    ESP_LOGI(TAG, "RGB probe output enabled on GPIO%d", led_probe_gpio);
    return ESP_OK;
#else
    (void)gpio_num;
    return ESP_ERR_NOT_SUPPORTED;
#endif
}

int led_get_probe_gpio(void) {
#if CONFIG_BLINK_LED_STRIP_BACKEND_RMT
    return led_probe_gpio;
#else
    return -1;
#endif
}

void led_on(void) {
    rainbow_enabled = true;
    led_set_rgb(16, 16, 16);
}

void led_off(void) {
    rainbow_enabled = false;

    /* Set all LED off to clear all pixels */
    led_strip_clear(led_strip);
#if CONFIG_BLINK_LED_STRIP_BACKEND_RMT
    if (led_probe_strip) {
        led_strip_clear(led_probe_strip);
    }
#endif

    /* Update LED state */
    led_state = false;
}

void led_init(void) {
    ESP_LOGI(TAG, "example configured to blink addressable led!");
    /* LED strip initialization with the GPIO and pixels number*/
    led_strip_config_t strip_config = {
        .strip_gpio_num = CONFIG_BLINK_GPIO,
        .max_leds = 1, // at least one LED on board
        .led_model = LED_MODEL_WS2812,
        .color_component_format = LED_STRIP_COLOR_COMPONENT_FMT_GRB,
    };
#if CONFIG_BLINK_LED_STRIP_BACKEND_RMT
    led_strip_rmt_config_t rmt_config = {
        .resolution_hz = 10 * 1000 * 1000, // 10MHz
        .flags.with_dma = false,
    };
    ESP_ERROR_CHECK(
        led_strip_new_rmt_device(&strip_config, &rmt_config, &led_strip));
#elif CONFIG_BLINK_LED_STRIP_BACKEND_SPI
    led_strip_spi_config_t spi_config = {
        .spi_bus = SPI2_HOST,
        .flags.with_dma = true,
    };
    ESP_ERROR_CHECK(
        led_strip_new_spi_device(&strip_config, &spi_config, &led_strip));
#else
#error "unsupported LED strip backend"
#endif
    /* Set all LED off before the rainbow task starts updating it */
    led_strip_clear(led_strip);
    led_state = false;
    rainbow_enabled = true;
}

#elif CONFIG_BLINK_LED_GPIO

esp_err_t led_set_rgb(uint8_t red, uint8_t green, uint8_t blue) {
    led_state = (red || green || blue);
    return gpio_set_level(CONFIG_BLINK_GPIO, led_state);
}

esp_err_t led_set_probe_gpio(int gpio_num) {
    (void)gpio_num;
    return ESP_ERR_NOT_SUPPORTED;
}

int led_get_probe_gpio(void) { return -1; }

void led_on(void) {
    rainbow_enabled = true;
    led_set_rgb(1, 1, 1);
}

void led_off(void) {
    rainbow_enabled = false;
    led_set_rgb(0, 0, 0);
}

void led_init(void) {
    ESP_LOGI(TAG, "example configured to blink gpio led!");
    gpio_reset_pin(CONFIG_BLINK_GPIO);
    /* Set the GPIO as a push/pull output */
    gpio_set_direction(CONFIG_BLINK_GPIO, GPIO_MODE_OUTPUT);
    led_set_rgb(0, 0, 0);
    rainbow_enabled = true;
}

#else
#error "unsupported LED type"
#endif
