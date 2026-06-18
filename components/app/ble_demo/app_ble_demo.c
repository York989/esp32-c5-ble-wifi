/*
 * SPDX-FileCopyrightText: 2024-2025 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: Unlicense OR CC0-1.0
 */
/* Includes */
#include "app_ble_demo.h"

#include <stdbool.h>

#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "svc_ble.h"
#include "svc_heart_rate.h"
#include "svc_led_effect.h"

/* Defines */
#define APP_BLE_DEMO_HEART_RATE_TASK_PERIOD_MS 1000

/* Private variables */
static const char *const s_app_ble_demo_log_tag = "app_ble_demo";
static bool s_app_ble_demo_is_started;
static TaskHandle_t s_app_ble_demo_heart_rate_task_handle;

/* Private functions */
static void app_ble_demo_heart_rate_task(void *param) {
    (void)param;
    ESP_LOGI(s_app_ble_demo_log_tag, "heart-rate task started");

    while (1) {
        svc_heart_rate_update();
        ESP_LOGI(s_app_ble_demo_log_tag, "heart rate updated to %d",
                 svc_heart_rate_get_bpm());

        svc_ble_send_heart_rate_indication();
        vTaskDelay(pdMS_TO_TICKS(APP_BLE_DEMO_HEART_RATE_TASK_PERIOD_MS));
    }

    vTaskDelete(NULL);
}

/* Public functions */
esp_err_t app_ble_demo_start(void) {
    esp_err_t err;
    BaseType_t task_result;

    if (s_app_ble_demo_is_started) {
        return ESP_OK;
    }

    err = svc_led_effect_init();
    if (err != ESP_OK) {
        ESP_LOGE(s_app_ble_demo_log_tag,
                 "failed to initialize LED effect service: %s (%d)",
                 esp_err_to_name(err), err);
        return err;
    }

    err = svc_led_effect_start();
    if (err != ESP_OK) {
        ESP_LOGE(s_app_ble_demo_log_tag,
                 "failed to start LED effect service: %s (%d)",
                 esp_err_to_name(err), err);
        return err;
    }

    err = svc_ble_init();
    if (err != ESP_OK) {
        ESP_LOGE(s_app_ble_demo_log_tag, "failed to initialize BLE service: %s (%d)",
                 esp_err_to_name(err), err);
        return err;
    }

    err = svc_ble_start();
    if (err != ESP_OK) {
        ESP_LOGE(s_app_ble_demo_log_tag, "failed to start BLE service: %s (%d)",
                 esp_err_to_name(err), err);
        return err;
    }

    task_result = xTaskCreate(app_ble_demo_heart_rate_task, "Heart Rate",
                              4 * 1024, NULL, 5,
                              &s_app_ble_demo_heart_rate_task_handle);
    if (task_result != pdPASS) {
        s_app_ble_demo_heart_rate_task_handle = NULL;
        return ESP_ERR_NO_MEM;
    }

    s_app_ble_demo_is_started = true;
    return ESP_OK;
}
