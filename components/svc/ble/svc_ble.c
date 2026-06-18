/*
 * SPDX-FileCopyrightText: 2024-2025 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: Unlicense OR CC0-1.0
 */
/* Includes */
#include "svc_ble.h"

#include "svc_ble_internal.h"

#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "host/ble_hs.h"
#include "host/util/util.h"
#include "nimble/nimble_port.h"
#include "nimble/nimble_port_freertos.h"
#include "nvs_flash.h"
#include "sdkconfig.h"

/* Library function declarations */
void ble_store_config_init(void);

/* Private variables */
static const char *const s_svc_ble_log_tag = "svc_ble";
static TaskHandle_t s_svc_ble_host_task_handle;

/* Private functions */
static esp_err_t svc_ble_init_nvs(void) {
    esp_err_t err;

    err = nvs_flash_init();
    if (err == ESP_ERR_NVS_NO_FREE_PAGES ||
        err == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        err = nvs_flash_init();
    }

    return err;
}

static void svc_ble_host_config_init(void) {
    ble_hs_cfg.reset_cb = svc_ble_gap_on_stack_reset;
    ble_hs_cfg.sync_cb = svc_ble_gap_on_stack_sync;
    ble_hs_cfg.gatts_register_cb = svc_ble_gatt_register_cb;
    ble_hs_cfg.store_status_cb = ble_store_util_status_rr;

    ble_store_config_init();
}

static void svc_ble_host_task(void *param) {
    (void)param;
    ESP_LOGI(s_svc_ble_log_tag, "NimBLE host task started");

    nimble_port_run();

    s_svc_ble_host_task_handle = NULL;
    vTaskDelete(NULL);
}

/* Public functions */
esp_err_t svc_ble_init(void) {
    esp_err_t err;
    int rc;

    err = svc_ble_init_nvs();
    if (err != ESP_OK) {
        ESP_LOGE(s_svc_ble_log_tag, "failed to initialize NVS: %s (%d)",
                 esp_err_to_name(err), err);
        return err;
    }

    err = nimble_port_init();
    if (err != ESP_OK) {
        ESP_LOGE(s_svc_ble_log_tag,
                 "failed to initialize NimBLE stack: %s (%d)",
                 esp_err_to_name(err), err);
        return err;
    }

#if CONFIG_BT_NIMBLE_GAP_SERVICE
    rc = svc_ble_gap_init();
    if (rc != 0) {
        ESP_LOGE(s_svc_ble_log_tag,
                 "failed to initialize GAP service, error code: %d", rc);
        return ESP_FAIL;
    }
#endif

    rc = svc_ble_gatt_init();
    if (rc != 0) {
        ESP_LOGE(s_svc_ble_log_tag,
                 "failed to initialize GATT service, error code: %d", rc);
        return ESP_FAIL;
    }

    svc_ble_host_config_init();
    return ESP_OK;
}

esp_err_t svc_ble_start(void) {
    BaseType_t task_result;

    if (s_svc_ble_host_task_handle != NULL) {
        return ESP_OK;
    }

    task_result = xTaskCreate(svc_ble_host_task, "NimBLE Host", 4 * 1024, NULL,
                              5, &s_svc_ble_host_task_handle);
    if (task_result != pdPASS) {
        s_svc_ble_host_task_handle = NULL;
        return ESP_ERR_NO_MEM;
    }

    return ESP_OK;
}
