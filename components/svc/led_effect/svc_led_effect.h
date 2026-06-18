/*
 * SPDX-FileCopyrightText: 2024 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: Unlicense OR CC0-1.0
 */
#ifndef SVC_LED_EFFECT_H
#define SVC_LED_EFFECT_H

/* Includes */
#include <stdbool.h>

#include "esp_err.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Initialize the LED effect service and its low-level LED driver.
 *
 * @return ESP_OK on success, otherwise an ESP-IDF error code.
 */
esp_err_t svc_led_effect_init(void);

/**
 * @brief Start the LED effect task.
 *
 * The task runs the startup color diagnostic once and then keeps updating the
 * rainbow effect while the service is enabled.
 *
 * @return ESP_OK on success, otherwise an ESP-IDF error code.
 */
esp_err_t svc_led_effect_start(void);

/**
 * @brief Enable or disable the running LED effect.
 *
 * Disabling clears the LED immediately. Enabling resumes the rainbow effect.
 *
 * @param enabled true to enable, false to disable.
 * @return ESP_OK on success, otherwise an ESP-IDF error code.
 */
esp_err_t svc_led_effect_set_enabled(bool enabled);

/**
 * @brief Query whether the LED effect is currently enabled.
 *
 * @return true if enabled, false otherwise.
 */
bool svc_led_effect_is_enabled(void);

#ifdef __cplusplus
}
#endif

#endif // SVC_LED_EFFECT_H
