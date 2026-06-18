/*
 * SPDX-FileCopyrightText: 2024-2025 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: Unlicense OR CC0-1.0
 */
/* Includes */
#include "app_ble_demo.h"

#include "esp_log.h"

/* Private variables */
static const char *const s_app_main_log_tag = "app_main";

/* Public functions */
void app_main(void) {
    esp_err_t err;

    err = app_ble_demo_start();
    if (err != ESP_OK) {
        ESP_LOGE(s_app_main_log_tag, "failed to start application: %s (%d)",
                 esp_err_to_name(err), err);
    }
}
