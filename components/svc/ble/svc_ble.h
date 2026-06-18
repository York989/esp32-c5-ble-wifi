/*
 * SPDX-FileCopyrightText: 2024-2025 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: Unlicense OR CC0-1.0
 */
#ifndef SVC_BLE_H
#define SVC_BLE_H

/* Includes */
#include "esp_err.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Initialize NVS, NimBLE host, GAP, and GATT services.
 *
 * @return ESP_OK on success, otherwise an ESP-IDF error code.
 */
esp_err_t svc_ble_init(void);

/**
 * @brief Start the NimBLE host task.
 *
 * @return ESP_OK on success, otherwise an ESP-IDF error code.
 */
esp_err_t svc_ble_start(void);

/**
 * @brief Send a heart-rate indication when a central has subscribed.
 */
void svc_ble_send_heart_rate_indication(void);

#ifdef __cplusplus
}
#endif

#endif // SVC_BLE_H
