/*
 * SPDX-FileCopyrightText: 2024-2025 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: Unlicense OR CC0-1.0
 */
/* Includes */
#include "svc_led_effect.h"

#include <stdint.h>

#include "drv_rgb_led.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

/* Defines */
#define SVC_LED_EFFECT_TASK_PERIOD_MS 40
#define SVC_LED_EFFECT_HUE_STEP 3
#define SVC_LED_EFFECT_BRIGHTNESS 255
#define SVC_LED_EFFECT_DIAGNOSTIC_WHITE_HOLD_MS 5000
#define SVC_LED_EFFECT_DIAGNOSTIC_COLOR_HOLD_MS 1000

/* Private variables */
static const char *const s_svc_led_effect_log_tag = "svc_led_effect";
static volatile bool s_svc_led_effect_is_enabled = true;
static bool s_svc_led_effect_is_initialized;
static TaskHandle_t s_svc_led_effect_task_handle;

/* Private functions */
static void svc_led_effect_hsv_to_rgb(uint16_t hue, uint8_t saturation,
                                      uint8_t value, uint8_t *red,
                                      uint8_t *green, uint8_t *blue) {
    uint8_t region = hue / 60;
    uint8_t remainder = (hue - (region * 60)) * 255 / 60;
    uint8_t p = (value * (255 - saturation)) / 255;
    uint8_t q = (value * (255 - ((saturation * remainder) / 255))) / 255;
    uint8_t t =
        (value * (255 - ((saturation * (255 - remainder)) / 255))) / 255;

    switch (region) {
    case 0:
        *red = value;
        *green = t;
        *blue = p;
        break;
    case 1:
        *red = q;
        *green = value;
        *blue = p;
        break;
    case 2:
        *red = p;
        *green = value;
        *blue = t;
        break;
    case 3:
        *red = p;
        *green = q;
        *blue = value;
        break;
    case 4:
        *red = t;
        *green = p;
        *blue = value;
        break;
    default:
        *red = value;
        *green = p;
        *blue = q;
        break;
    }
}

static void svc_led_effect_show_diagnostic_step(const char *name, uint8_t red,
                                                uint8_t green, uint8_t blue,
                                                TickType_t hold_ticks) {
    esp_err_t err;

    ESP_LOGI(s_svc_led_effect_log_tag, "diagnostic %s rgb=(%u,%u,%u)", name,
             red, green, blue);
    err = drv_rgb_led_set_rgb(red, green, blue);
    ESP_LOGI(s_svc_led_effect_log_tag, "diagnostic %s result=%s (%d)", name,
             esp_err_to_name(err), err);
    vTaskDelay(hold_ticks);
}

static void svc_led_effect_show_startup_diagnostic(void) {
    svc_led_effect_show_diagnostic_step(
        "full white", 255, 255, 255,
        pdMS_TO_TICKS(SVC_LED_EFFECT_DIAGNOSTIC_WHITE_HOLD_MS));

    svc_led_effect_show_diagnostic_step(
        "red", 255, 0, 0,
        pdMS_TO_TICKS(SVC_LED_EFFECT_DIAGNOSTIC_COLOR_HOLD_MS));

    svc_led_effect_show_diagnostic_step(
        "green", 0, 255, 0,
        pdMS_TO_TICKS(SVC_LED_EFFECT_DIAGNOSTIC_COLOR_HOLD_MS));

    svc_led_effect_show_diagnostic_step(
        "blue", 0, 0, 255,
        pdMS_TO_TICKS(SVC_LED_EFFECT_DIAGNOSTIC_COLOR_HOLD_MS));
}

static void svc_led_effect_rainbow_task(void *param) {
    uint16_t hue = 0;
    uint8_t red = 0;
    uint8_t green = 0;
    uint8_t blue = 0;

    (void)param;
    ESP_LOGI(s_svc_led_effect_log_tag, "LED effect task started");

    svc_led_effect_show_startup_diagnostic();

    while (1) {
        if (svc_led_effect_is_enabled()) {
            svc_led_effect_hsv_to_rgb(hue, 255, SVC_LED_EFFECT_BRIGHTNESS,
                                      &red, &green, &blue);
            (void)drv_rgb_led_set_rgb(red, green, blue);
            hue = (hue + SVC_LED_EFFECT_HUE_STEP) % 360;
        }

        vTaskDelay(pdMS_TO_TICKS(SVC_LED_EFFECT_TASK_PERIOD_MS));
    }

    vTaskDelete(NULL);
}

/* Public functions */
esp_err_t svc_led_effect_init(void) {
    esp_err_t err;

    if (s_svc_led_effect_is_initialized) {
        return ESP_OK;
    }

    err = drv_rgb_led_init();
    if (err != ESP_OK) {
        ESP_LOGE(s_svc_led_effect_log_tag,
                 "failed to initialize RGB LED driver: %s (%d)",
                 esp_err_to_name(err), err);
        return err;
    }

    s_svc_led_effect_is_enabled = true;
    s_svc_led_effect_is_initialized = true;
    return ESP_OK;
}

esp_err_t svc_led_effect_start(void) {
    BaseType_t task_result;

    if (!s_svc_led_effect_is_initialized) {
        return ESP_ERR_INVALID_STATE;
    }

    if (s_svc_led_effect_task_handle != NULL) {
        return ESP_OK;
    }

    task_result = xTaskCreate(svc_led_effect_rainbow_task, "LED Effect",
                              2 * 1024, NULL, 4,
                              &s_svc_led_effect_task_handle);
    if (task_result != pdPASS) {
        s_svc_led_effect_task_handle = NULL;
        return ESP_ERR_NO_MEM;
    }

    return ESP_OK;
}

esp_err_t svc_led_effect_set_enabled(bool enabled) {
    s_svc_led_effect_is_enabled = enabled;

    if (!enabled) {
        return drv_rgb_led_clear();
    }

    return drv_rgb_led_set_rgb(16, 16, 16);
}

bool svc_led_effect_is_enabled(void) { return s_svc_led_effect_is_enabled; }
