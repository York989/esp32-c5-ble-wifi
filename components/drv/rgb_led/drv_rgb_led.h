/*
 * SPDX-FileCopyrightText: 2024 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: Unlicense OR CC0-1.0
 */
#ifndef DRV_RGB_LED_H
#define DRV_RGB_LED_H

/* Includes */
#include <stdint.h>

#include "esp_err.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Initialize the board RGB LED hardware.
 *
 * The driver owns the low-level WS2812/GPIO configuration. Call this once
 * before writing RGB values.
 *
 * @return ESP_OK on success, otherwise an ESP-IDF error code.
 */
esp_err_t drv_rgb_led_init(void);

/**
 * @brief Set the RGB LED color.
 *
 * @param red Red channel value, 0-255.
 * @param green Green channel value, 0-255.
 * @param blue Blue channel value, 0-255.
 * @return ESP_OK on success, otherwise an ESP-IDF error code.
 */
esp_err_t drv_rgb_led_set_rgb(uint8_t red, uint8_t green, uint8_t blue);

/**
 * @brief Turn off the RGB LED and clear any pending pixel value.
 *
 * @return ESP_OK on success, otherwise an ESP-IDF error code.
 */
esp_err_t drv_rgb_led_clear(void);

#ifdef __cplusplus
}
#endif

#endif // DRV_RGB_LED_H
