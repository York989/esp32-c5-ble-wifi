/*
 * SPDX-FileCopyrightText: 2024-2025 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: Unlicense OR CC0-1.0
 */
/* Includes */
#include "svc_ble_internal.h"

#include <stdio.h>
#include <string.h>

#include "esp_log.h"
#include "host/ble_gap.h"
#include "host/ble_hs.h"
#include "host/ble_uuid.h"
#include "host/util/util.h"
#include "services/gap/ble_svc_gap.h"

/* Defines */
#define SVC_BLE_GAP_APPEARANCE_GENERIC_TAG 0x0200
#define SVC_BLE_GAP_URI_PREFIX_HTTPS 0x17
#define SVC_BLE_GAP_LE_ROLE_PERIPHERAL 0x00
#define SVC_BLE_DEVICE_NAME "NimBLE_GATT"

/* Private variables */
static const char *const s_svc_ble_gap_log_tag = "svc_ble_gap";
static uint8_t s_svc_ble_gap_own_addr_type;
static uint8_t s_svc_ble_gap_device_addr[6];
static uint8_t s_svc_ble_gap_device_uri[] = {
    SVC_BLE_GAP_URI_PREFIX_HTTPS, '/', '/', 'e', 's', 'p', 'r', 'e',
    's', 's', 'i', 'f', '.', 'c', 'o', 'm'};

/* Private functions */
static void svc_ble_gap_format_addr(char *addr_str, const uint8_t addr[]) {
    snprintf(addr_str, 18, "%02X:%02X:%02X:%02X:%02X:%02X", addr[0], addr[1],
             addr[2], addr[3], addr[4], addr[5]);
}

static void svc_ble_gap_print_conn_desc(const struct ble_gap_conn_desc *desc) {
    char addr_str[18] = {0};

    ESP_LOGI(s_svc_ble_gap_log_tag, "connection handle: %d", desc->conn_handle);

    svc_ble_gap_format_addr(addr_str, desc->our_id_addr.val);
    ESP_LOGI(s_svc_ble_gap_log_tag, "device id address: type=%d, value=%s",
             desc->our_id_addr.type, addr_str);

    svc_ble_gap_format_addr(addr_str, desc->peer_id_addr.val);
    ESP_LOGI(s_svc_ble_gap_log_tag, "peer id address: type=%d, value=%s",
             desc->peer_id_addr.type, addr_str);

    ESP_LOGI(s_svc_ble_gap_log_tag,
             "conn_itvl=%d, conn_latency=%d, supervision_timeout=%d, "
             "encrypted=%d, authenticated=%d, bonded=%d",
             desc->conn_itvl, desc->conn_latency, desc->supervision_timeout,
             desc->sec_state.encrypted, desc->sec_state.authenticated,
             desc->sec_state.bonded);
}

static int svc_ble_gap_event_handler(struct ble_gap_event *event, void *arg) {
    int rc = 0;
    struct ble_gap_conn_desc desc;

    (void)arg;

    switch (event->type) {
    case BLE_GAP_EVENT_CONNECT:
        ESP_LOGI(s_svc_ble_gap_log_tag, "connection %s; status=%d",
                 event->connect.status == 0 ? "established" : "failed",
                 event->connect.status);

        if (event->connect.status == 0) {
            rc = ble_gap_conn_find(event->connect.conn_handle, &desc);
            if (rc != 0) {
                ESP_LOGE(s_svc_ble_gap_log_tag,
                         "failed to find connection by handle, error code: %d",
                         rc);
                return rc;
            }

            svc_ble_gap_print_conn_desc(&desc);

            struct ble_gap_upd_params params = {
                .itvl_min = desc.conn_itvl,
                .itvl_max = desc.conn_itvl,
                .latency = 3,
                .supervision_timeout = desc.supervision_timeout};
            rc = ble_gap_update_params(event->connect.conn_handle, &params);
            if (rc != 0) {
                ESP_LOGE(s_svc_ble_gap_log_tag,
                         "failed to update connection parameters, error code: %d",
                         rc);
                return rc;
            }
        } else {
            svc_ble_gap_start_advertising();
        }
        return rc;

    case BLE_GAP_EVENT_DISCONNECT:
        ESP_LOGI(s_svc_ble_gap_log_tag, "disconnected from peer; reason=%d",
                 event->disconnect.reason);
        svc_ble_gatt_reset_heart_rate_subscription();
        svc_ble_gap_start_advertising();
        return rc;

    case BLE_GAP_EVENT_CONN_UPDATE:
        ESP_LOGI(s_svc_ble_gap_log_tag, "connection updated; status=%d",
                 event->conn_update.status);

        rc = ble_gap_conn_find(event->conn_update.conn_handle, &desc);
        if (rc != 0) {
            ESP_LOGE(s_svc_ble_gap_log_tag,
                     "failed to find connection by handle, error code: %d",
                     rc);
            return rc;
        }
        svc_ble_gap_print_conn_desc(&desc);
        return rc;

    case BLE_GAP_EVENT_ADV_COMPLETE:
        ESP_LOGI(s_svc_ble_gap_log_tag, "advertise complete; reason=%d",
                 event->adv_complete.reason);
        svc_ble_gap_start_advertising();
        return rc;

    case BLE_GAP_EVENT_NOTIFY_TX:
        if ((event->notify_tx.status != 0) &&
            (event->notify_tx.status != BLE_HS_EDONE)) {
            ESP_LOGI(s_svc_ble_gap_log_tag,
                     "notify event; conn_handle=%d attr_handle=%d "
                     "status=%d is_indication=%d",
                     event->notify_tx.conn_handle, event->notify_tx.attr_handle,
                     event->notify_tx.status, event->notify_tx.indication);
        }
        return rc;

    case BLE_GAP_EVENT_SUBSCRIBE:
        ESP_LOGI(s_svc_ble_gap_log_tag,
                 "subscribe event; conn_handle=%d attr_handle=%d "
                 "reason=%d prevn=%d curn=%d previ=%d curi=%d",
                 event->subscribe.conn_handle, event->subscribe.attr_handle,
                 event->subscribe.reason, event->subscribe.prev_notify,
                 event->subscribe.cur_notify, event->subscribe.prev_indicate,
                 event->subscribe.cur_indicate);
        svc_ble_gatt_subscribe_cb(event);
        return rc;

    case BLE_GAP_EVENT_MTU:
        ESP_LOGI(s_svc_ble_gap_log_tag, "mtu update event; conn_handle=%d cid=%d mtu=%d",
                 event->mtu.conn_handle, event->mtu.channel_id,
                 event->mtu.value);
        return rc;
    }

    return rc;
}

static void svc_ble_gap_start_advertising_impl(void) {
    int rc;
    const char *name;
    struct ble_hs_adv_fields adv_fields = {0};
    struct ble_hs_adv_fields rsp_fields = {0};
    struct ble_gap_adv_params adv_params = {0};

    adv_fields.flags = BLE_HS_ADV_F_DISC_GEN | BLE_HS_ADV_F_BREDR_UNSUP;

    name = ble_svc_gap_device_name();
    adv_fields.name = (uint8_t *)name;
    adv_fields.name_len = strlen(name);
    adv_fields.name_is_complete = 1;

    adv_fields.tx_pwr_lvl = BLE_HS_ADV_TX_PWR_LVL_AUTO;
    adv_fields.tx_pwr_lvl_is_present = 1;

    adv_fields.appearance = SVC_BLE_GAP_APPEARANCE_GENERIC_TAG;
    adv_fields.appearance_is_present = 1;

    adv_fields.le_role = SVC_BLE_GAP_LE_ROLE_PERIPHERAL;
    adv_fields.le_role_is_present = 1;

    rc = ble_gap_adv_set_fields(&adv_fields);
    if (rc != 0) {
        ESP_LOGE(s_svc_ble_gap_log_tag,
                 "failed to set advertising data, error code: %d", rc);
        return;
    }

    rsp_fields.device_addr = s_svc_ble_gap_device_addr;
    rsp_fields.device_addr_type = s_svc_ble_gap_own_addr_type;
    rsp_fields.device_addr_is_present = 1;

    rsp_fields.uri = s_svc_ble_gap_device_uri;
    rsp_fields.uri_len = sizeof(s_svc_ble_gap_device_uri);

    rsp_fields.adv_itvl = BLE_GAP_ADV_ITVL_MS(500);
    rsp_fields.adv_itvl_is_present = 1;

    rc = ble_gap_adv_rsp_set_fields(&rsp_fields);
    if (rc != 0) {
        ESP_LOGE(s_svc_ble_gap_log_tag,
                 "failed to set scan response data, error code: %d", rc);
        return;
    }

    adv_params.conn_mode = BLE_GAP_CONN_MODE_UND;
    adv_params.disc_mode = BLE_GAP_DISC_MODE_GEN;
    adv_params.itvl_min = BLE_GAP_ADV_ITVL_MS(500);
    adv_params.itvl_max = BLE_GAP_ADV_ITVL_MS(510);

    rc = ble_gap_adv_start(s_svc_ble_gap_own_addr_type, NULL, BLE_HS_FOREVER,
                           &adv_params, svc_ble_gap_event_handler, NULL);
    if (rc != 0) {
        ESP_LOGE(s_svc_ble_gap_log_tag,
                 "failed to start advertising, error code: %d", rc);
        return;
    }

    ESP_LOGI(s_svc_ble_gap_log_tag, "advertising started");
}

static int svc_ble_gap_prepare_address(void) {
    int rc;
    char addr_str[18] = {0};

    rc = ble_hs_util_ensure_addr(0);
    if (rc != 0) {
        ESP_LOGE(s_svc_ble_gap_log_tag,
                 "device does not have any available BLE address");
        return rc;
    }

    rc = ble_hs_id_infer_auto(0, &s_svc_ble_gap_own_addr_type);
    if (rc != 0) {
        ESP_LOGE(s_svc_ble_gap_log_tag,
                 "failed to infer address type, error code: %d", rc);
        return rc;
    }

    rc = ble_hs_id_copy_addr(s_svc_ble_gap_own_addr_type,
                             s_svc_ble_gap_device_addr, NULL);
    if (rc != 0) {
        ESP_LOGE(s_svc_ble_gap_log_tag,
                 "failed to copy device address, error code: %d", rc);
        return rc;
    }

    svc_ble_gap_format_addr(addr_str, s_svc_ble_gap_device_addr);
    ESP_LOGI(s_svc_ble_gap_log_tag, "device address: %s", addr_str);
    return 0;
}

/* Public functions */
void svc_ble_gap_on_stack_reset(int reason) {
    ESP_LOGI(s_svc_ble_gap_log_tag, "NimBLE stack reset, reason: %d", reason);
}

void svc_ble_gap_on_stack_sync(void) {
    if (svc_ble_gap_prepare_address() != 0) {
        return;
    }

    svc_ble_gap_start_advertising_impl();
}

int svc_ble_gap_init(void) {
    int rc;

    ble_svc_gap_init();

    rc = ble_svc_gap_device_name_set(SVC_BLE_DEVICE_NAME);
    if (rc != 0) {
        ESP_LOGE(s_svc_ble_gap_log_tag,
                 "failed to set device name to %s, error code: %d",
                 SVC_BLE_DEVICE_NAME, rc);
        return rc;
    }

    return 0;
}

void svc_ble_gap_start_advertising(void) { svc_ble_gap_start_advertising_impl(); }
