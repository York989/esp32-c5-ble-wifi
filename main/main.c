/*
 * SPDX-FileCopyrightText: 2024-2025 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: Unlicense OR CC0-1.0
 */
/* Includes */
#include "common.h"
#include "gap.h"
#include "gatt_svc.h"
#include "heart_rate.h"
#include "led.h"

/* Library function declarations */
void ble_store_config_init(void);

/* Private function declarations */
static void on_stack_reset(int reason);
static void on_stack_sync(void);
static void nimble_host_config_init(void);
static void nimble_host_task(void *param);
static void hsv_to_rgb(uint16_t hue, uint8_t saturation, uint8_t value,
                       uint8_t *red, uint8_t *green, uint8_t *blue);
static void led_diagnostic_set_rgb(const char *name, uint8_t red,
                                   uint8_t green, uint8_t blue,
                                   TickType_t hold_ticks);
static void led_rotate_probe_gpio(TickType_t now);
static void led_startup_diagnostic(void);
static void led_rainbow_task(void *param);

/* Private defines */
#define LED_RAINBOW_TASK_PERIOD_MS 40
#define LED_RAINBOW_HUE_STEP 3
#define LED_RAINBOW_BRIGHTNESS 255
#define LED_DIAGNOSTIC_WHITE_HOLD_MS 5000
#define LED_DIAGNOSTIC_COLOR_HOLD_MS 1000
#define LED_PROBE_ROTATION_MS 5000

static const int led_probe_gpio_candidates[] = {23, 24, 4, 5};

/* Private functions */
/*
 *  Stack event callback functions
 *      - on_stack_reset is called when host resets BLE stack due to errors
 *      - on_stack_sync is called when host has synced with controller
 */
static void on_stack_reset(int reason) {
    /* On reset, print reset reason to console */
    ESP_LOGI(TAG, "nimble stack reset, reset reason: %d", reason);
}

static void on_stack_sync(void) {
    /* On stack sync, do advertising initialization */
    adv_init();
}

static void nimble_host_config_init(void) {
    /* Set host callbacks */
    ble_hs_cfg.reset_cb = on_stack_reset;
    ble_hs_cfg.sync_cb = on_stack_sync;
    ble_hs_cfg.gatts_register_cb = gatt_svr_register_cb;
    ble_hs_cfg.store_status_cb = ble_store_util_status_rr;

    /* Store host configuration */
    ble_store_config_init();
}

static void nimble_host_task(void *param) {
    /* Task entry log */
    ESP_LOGI(TAG, "nimble host task has been started!");

    /* This function won't return until nimble_port_stop() is executed */
    nimble_port_run();

    /* Clean up at exit */
    vTaskDelete(NULL);
}

static void hsv_to_rgb(uint16_t hue, uint8_t saturation, uint8_t value,
                       uint8_t *red, uint8_t *green, uint8_t *blue) {
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

static void led_diagnostic_set_rgb(const char *name, uint8_t red,
                                   uint8_t green, uint8_t blue,
                                   TickType_t hold_ticks) {
    esp_err_t ret;

    ESP_LOGI(TAG, "led diagnostic: %s rgb=(%u,%u,%u)", name, red, green,
             blue);
    ret = led_set_rgb(red, green, blue);
    ESP_LOGI(TAG, "led diagnostic: %s send result=%s (%d)", name,
             esp_err_to_name(ret), ret);
    vTaskDelay(hold_ticks);
}

static void led_startup_diagnostic(void) {
    led_diagnostic_set_rgb("full white", 255, 255, 255,
                           pdMS_TO_TICKS(LED_DIAGNOSTIC_WHITE_HOLD_MS));

    led_diagnostic_set_rgb("red", 255, 0, 0,
                           pdMS_TO_TICKS(LED_DIAGNOSTIC_COLOR_HOLD_MS));

    led_diagnostic_set_rgb("green", 0, 255, 0,
                           pdMS_TO_TICKS(LED_DIAGNOSTIC_COLOR_HOLD_MS));

    led_diagnostic_set_rgb("blue", 0, 0, 255,
                           pdMS_TO_TICKS(LED_DIAGNOSTIC_COLOR_HOLD_MS));
}

static void led_rotate_probe_gpio(TickType_t now) {
    static TickType_t next_switch_tick;
    static size_t probe_index;
    esp_err_t ret;
    int gpio;

    if (now < next_switch_tick) {
        return;
    }

    gpio = led_probe_gpio_candidates[probe_index];
    ret = led_set_probe_gpio(gpio);
    ESP_LOGI(TAG, "led diagnostic: active RGB probe GPIO%d result=%s (%d)",
             gpio, esp_err_to_name(ret), ret);

    probe_index = (probe_index + 1) %
                  (sizeof(led_probe_gpio_candidates) /
                   sizeof(led_probe_gpio_candidates[0]));
    next_switch_tick = now + pdMS_TO_TICKS(LED_PROBE_ROTATION_MS);
}

static void led_rainbow_task(void *param) {
    uint16_t hue = 0;
    uint8_t red = 0;
    uint8_t green = 0;
    uint8_t blue = 0;
    TickType_t now;

    ESP_LOGI(TAG, "led rainbow task has been started!");

    led_startup_diagnostic();

    while (1) {
        now = xTaskGetTickCount();
        led_rotate_probe_gpio(now);

        if (led_is_rainbow_enabled()) {
            hsv_to_rgb(hue, 255, LED_RAINBOW_BRIGHTNESS, &red, &green, &blue);
            led_set_rgb(red, green, blue);
            hue = (hue + LED_RAINBOW_HUE_STEP) % 360;
        }

        vTaskDelay(pdMS_TO_TICKS(LED_RAINBOW_TASK_PERIOD_MS));
    }

    vTaskDelete(NULL);
}

static void heart_rate_task(void *param) {
    /* Task entry log */
    ESP_LOGI(TAG, "heart rate task has been started!");

    /* Loop forever */
    while (1) {
        /* Update heart rate value every 1 second */
        update_heart_rate();
        ESP_LOGI(TAG, "heart rate updated to %d", get_heart_rate());

        /* Send heart rate indication if enabled */
        send_heart_rate_indication();

        /* Sleep */
        vTaskDelay(HEART_RATE_TASK_PERIOD);
    }

    /* Clean up at exit */
    vTaskDelete(NULL);
}

void app_main(void) {
    /* Local variables */
    BaseType_t rc = 0;
    esp_err_t ret;

    /* LED initialization */
    led_init();

    rc = xTaskCreate(led_rainbow_task, "LED Rainbow", 2 * 1024, NULL, 4, NULL);
    if (rc != pdPASS) {
        ESP_LOGE(TAG, "failed to create LED rainbow task");
        return;
    }

    /*
     * NVS flash initialization
     * Dependency of BLE stack to store configurations
     */
    ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES ||
        ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "failed to initialize nvs flash, error code: %d ", ret);
        return;
    }

    /* NimBLE stack initialization */
    ret = nimble_port_init();
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "failed to initialize nimble stack, error code: %d ",
                 ret);
        return;
    }

#if CONFIG_BT_NIMBLE_GAP_SERVICE
    /* GAP service initialization */
    rc = gap_init();
    if (rc != 0) {
        ESP_LOGE(TAG, "failed to initialize GAP service, error code: %d", rc);
        return;
    }
#endif

    /* GATT server initialization */
    rc = gatt_svc_init();
    if (rc != 0) {
        ESP_LOGE(TAG, "failed to initialize GATT server, error code: %d", rc);
        return;
    }

    /* NimBLE host configuration initialization */
    nimble_host_config_init();

    /* Start NimBLE host task thread and return */
    rc = xTaskCreate(nimble_host_task, "NimBLE Host", 4 * 1024, NULL,
                                5, NULL);
    if (rc != pdPASS) {
        ESP_LOGE(TAG, "failed to create NimBLE host task");
        return;
    }

    rc = xTaskCreate(heart_rate_task, "Heart Rate", 4 * 1024, NULL, 5, NULL);
    if (rc != pdPASS) {
        ESP_LOGE(TAG, "failed to create heart rate task");
        return;
    }
    return;
}
