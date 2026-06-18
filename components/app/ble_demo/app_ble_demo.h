/*
 * SPDX-FileCopyrightText: 2024-2025 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: Unlicense OR CC0-1.0
 */
#ifndef APP_BLE_DEMO_H
#define APP_BLE_DEMO_H

/* Includes */
#include "esp_err.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Start the BLE demo application.
 *
 * This initializes LED effects, BLE services, and the mock heart-rate update
 * task. It is the only application entry used by app_main.
 *
 * @return ESP_OK on success, otherwise an ESP-IDF error code.
 */
esp_err_t app_ble_demo_start(void);

#ifdef __cplusplus
}
#endif

#endif // APP_BLE_DEMO_H
