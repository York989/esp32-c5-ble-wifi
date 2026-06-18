/*
 * SPDX-FileCopyrightText: 2024-2025 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: Unlicense OR CC0-1.0
 */
/* Includes */
#include "svc_ble_internal.h"

#include <assert.h>

#include "esp_log.h"
#include "host/ble_gap.h"
#include "host/ble_gatt.h"
#include "host/ble_hs.h"
#include "host/ble_uuid.h"
#include "host/util/util.h"
#include "services/gatt/ble_svc_gatt.h"

#include "svc_heart_rate.h"
#include "svc_led_effect.h"

/* Private function declarations */
static int svc_ble_gatt_heart_rate_chr_access(uint16_t conn_handle,
                                              uint16_t attr_handle,
                                              struct ble_gatt_access_ctxt *ctxt,
                                              void *arg);
static int svc_ble_gatt_led_chr_access(uint16_t conn_handle,
                                       uint16_t attr_handle,
                                       struct ble_gatt_access_ctxt *ctxt,
                                       void *arg);

/* Private variables */
static const char *const s_svc_ble_gatt_log_tag = "svc_ble_gatt";
static const ble_uuid16_t s_svc_ble_gatt_heart_rate_service_uuid =
    BLE_UUID16_INIT(0x180D);
static uint8_t s_svc_ble_gatt_heart_rate_chr_value[2] = {0};
static uint16_t s_svc_ble_gatt_heart_rate_chr_value_handle;
static const ble_uuid16_t s_svc_ble_gatt_heart_rate_chr_uuid =
    BLE_UUID16_INIT(0x2A37);
static uint16_t s_svc_ble_gatt_heart_rate_conn_handle =
    BLE_HS_CONN_HANDLE_NONE;
static bool s_svc_ble_gatt_heart_rate_conn_handle_is_valid;
static bool s_svc_ble_gatt_heart_rate_indication_enabled;

static const ble_uuid16_t s_svc_ble_gatt_led_service_uuid = BLE_UUID16_INIT(0x1815);
static uint16_t s_svc_ble_gatt_led_chr_value_handle;
static const ble_uuid128_t s_svc_ble_gatt_led_chr_uuid = BLE_UUID128_INIT(
    0x23, 0xd1, 0xbc, 0xea, 0x5f, 0x78, 0x23, 0x15, 0xde, 0xef, 0x12, 0x12,
    0x25, 0x15, 0x00, 0x00);

static const struct ble_gatt_svc_def s_svc_ble_gatt_services[] = {
    {.type = BLE_GATT_SVC_TYPE_PRIMARY,
     .uuid = &s_svc_ble_gatt_heart_rate_service_uuid.u,
     .characteristics =
         (struct ble_gatt_chr_def[]){{
                                         .uuid = &s_svc_ble_gatt_heart_rate_chr_uuid.u,
                                         .access_cb =
                                             svc_ble_gatt_heart_rate_chr_access,
                                         .flags = BLE_GATT_CHR_F_READ |
                                                  BLE_GATT_CHR_F_INDICATE,
                                         .val_handle =
                                             &s_svc_ble_gatt_heart_rate_chr_value_handle,
                                     },
                                     {
                                         0,
                                     }}},
    {.type = BLE_GATT_SVC_TYPE_PRIMARY,
     .uuid = &s_svc_ble_gatt_led_service_uuid.u,
     .characteristics =
         (struct ble_gatt_chr_def[]){{
                                         .uuid = &s_svc_ble_gatt_led_chr_uuid.u,
                                         .access_cb = svc_ble_gatt_led_chr_access,
                                         .flags = BLE_GATT_CHR_F_WRITE,
                                         .val_handle = &s_svc_ble_gatt_led_chr_value_handle,
                                     },
                                     {0}}},
    {0},
};

/* Private functions */
static int svc_ble_gatt_heart_rate_chr_access(uint16_t conn_handle,
                                               uint16_t attr_handle,
                                               struct ble_gatt_access_ctxt *ctxt,
                                               void *arg) {
    int rc = 0;

    (void)arg;

    switch (ctxt->op) {
    case BLE_GATT_ACCESS_OP_READ_CHR:
        if (conn_handle != BLE_HS_CONN_HANDLE_NONE) {
            ESP_LOGI(s_svc_ble_gatt_log_tag,
                     "heart-rate read; conn_handle=%d attr_handle=%d",
                     conn_handle, attr_handle);
        } else {
            ESP_LOGI(s_svc_ble_gatt_log_tag,
                     "heart-rate read by NimBLE; attr_handle=%d", attr_handle);
        }

        if (attr_handle == s_svc_ble_gatt_heart_rate_chr_value_handle) {
            s_svc_ble_gatt_heart_rate_chr_value[1] = svc_heart_rate_get_bpm();
            rc = os_mbuf_append(ctxt->om, &s_svc_ble_gatt_heart_rate_chr_value,
                                sizeof(s_svc_ble_gatt_heart_rate_chr_value));
            return rc == 0 ? 0 : BLE_ATT_ERR_INSUFFICIENT_RES;
        }
        goto error;

    default:
        goto error;
    }

error:
    ESP_LOGE(s_svc_ble_gatt_log_tag,
             "unexpected access to heart-rate characteristic, opcode: %d",
             ctxt->op);
    return BLE_ATT_ERR_UNLIKELY;
}

static int svc_ble_gatt_led_chr_access(uint16_t conn_handle, uint16_t attr_handle,
                                       struct ble_gatt_access_ctxt *ctxt,
                                       void *arg) {
    int rc = 0;

    (void)arg;

    switch (ctxt->op) {
    case BLE_GATT_ACCESS_OP_WRITE_CHR:
        if (conn_handle != BLE_HS_CONN_HANDLE_NONE) {
            ESP_LOGI(s_svc_ble_gatt_log_tag,
                     "LED write; conn_handle=%d attr_handle=%d", conn_handle,
                     attr_handle);
        } else {
            ESP_LOGI(s_svc_ble_gatt_log_tag,
                     "LED write by NimBLE; attr_handle=%d", attr_handle);
        }

        if (attr_handle == s_svc_ble_gatt_led_chr_value_handle) {
            if (ctxt->om->om_len == 1) {
                rc = svc_led_effect_set_enabled(ctxt->om->om_data[0] != 0);
                if (rc != ESP_OK) {
                    ESP_LOGE(s_svc_ble_gatt_log_tag,
                             "failed to update LED effect from GATT: %s (%d)",
                             esp_err_to_name(rc), rc);
                    return BLE_ATT_ERR_UNLIKELY;
                }

                ESP_LOGI(s_svc_ble_gatt_log_tag, "LED effect %s",
                         ctxt->om->om_data[0] ? "enabled" : "disabled");
                return 0;
            }
            goto error;
        }
        goto error;

    default:
        goto error;
    }

error:
    ESP_LOGE(s_svc_ble_gatt_log_tag,
             "unexpected access to LED characteristic, opcode: %d",
             ctxt->op);
    return BLE_ATT_ERR_UNLIKELY;
}

/* Public functions */
void svc_ble_send_heart_rate_indication(void) {
    if (s_svc_ble_gatt_heart_rate_indication_enabled &&
        s_svc_ble_gatt_heart_rate_conn_handle_is_valid) {
        ble_gatts_indicate(s_svc_ble_gatt_heart_rate_conn_handle,
                           s_svc_ble_gatt_heart_rate_chr_value_handle);
        ESP_LOGI(s_svc_ble_gatt_log_tag, "heart-rate indication sent");
    }
}

void svc_ble_gatt_register_cb(struct ble_gatt_register_ctxt *ctxt, void *arg) {
    char buf[BLE_UUID_STR_LEN];

    (void)arg;

    switch (ctxt->op) {
    case BLE_GATT_REGISTER_OP_SVC:
        ESP_LOGD(s_svc_ble_gatt_log_tag, "registered service %s with handle=%d",
                 ble_uuid_to_str(ctxt->svc.svc_def->uuid, buf),
                 ctxt->svc.handle);
        break;
    case BLE_GATT_REGISTER_OP_CHR:
        ESP_LOGD(s_svc_ble_gatt_log_tag,
                 "registering characteristic %s with def_handle=%d val_handle=%d",
                 ble_uuid_to_str(ctxt->chr.chr_def->uuid, buf),
                 ctxt->chr.def_handle, ctxt->chr.val_handle);
        break;
    case BLE_GATT_REGISTER_OP_DSC:
        ESP_LOGD(s_svc_ble_gatt_log_tag, "registering descriptor %s with handle=%d",
                 ble_uuid_to_str(ctxt->dsc.dsc_def->uuid, buf),
                 ctxt->dsc.handle);
        break;
    default:
        assert(0);
        break;
    }
}

void svc_ble_gatt_subscribe_cb(struct ble_gap_event *event) {
    if (event->subscribe.conn_handle != BLE_HS_CONN_HANDLE_NONE) {
        ESP_LOGI(s_svc_ble_gatt_log_tag,
                 "subscribe event; conn_handle=%d attr_handle=%d",
                 event->subscribe.conn_handle, event->subscribe.attr_handle);
    } else {
        ESP_LOGI(s_svc_ble_gatt_log_tag,
                 "subscribe by NimBLE; attr_handle=%d",
                 event->subscribe.attr_handle);
    }

    if (event->subscribe.attr_handle ==
        s_svc_ble_gatt_heart_rate_chr_value_handle) {
        s_svc_ble_gatt_heart_rate_conn_handle = event->subscribe.conn_handle;
        s_svc_ble_gatt_heart_rate_conn_handle_is_valid = true;
        s_svc_ble_gatt_heart_rate_indication_enabled =
            event->subscribe.cur_indicate;
    }
}

void svc_ble_gatt_reset_heart_rate_subscription(void) {
    s_svc_ble_gatt_heart_rate_conn_handle = BLE_HS_CONN_HANDLE_NONE;
    s_svc_ble_gatt_heart_rate_conn_handle_is_valid = false;
    s_svc_ble_gatt_heart_rate_indication_enabled = false;
}

int svc_ble_gatt_init(void) {
    int rc;

    ble_svc_gatt_init();

    rc = ble_gatts_count_cfg(s_svc_ble_gatt_services);
    if (rc != 0) {
        return rc;
    }

    rc = ble_gatts_add_svcs(s_svc_ble_gatt_services);
    if (rc != 0) {
        return rc;
    }

    return 0;
}
