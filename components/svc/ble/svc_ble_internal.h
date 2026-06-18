/*
 * SPDX-FileCopyrightText: 2024-2025 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: Unlicense OR CC0-1.0
 */
#ifndef SVC_BLE_INTERNAL_H
#define SVC_BLE_INTERNAL_H

/* Includes */
#include "host/ble_gap.h"
#include "host/ble_gatt.h"

#ifdef __cplusplus
extern "C" {
#endif

/* GAP internal API */
int svc_ble_gap_init(void);
void svc_ble_gap_on_stack_reset(int reason);
void svc_ble_gap_on_stack_sync(void);
void svc_ble_gap_start_advertising(void);

/* GATT internal API */
int svc_ble_gatt_init(void);
void svc_ble_gatt_register_cb(struct ble_gatt_register_ctxt *ctxt, void *arg);
void svc_ble_gatt_subscribe_cb(struct ble_gap_event *event);
void svc_ble_gatt_reset_heart_rate_subscription(void);

#ifdef __cplusplus
}
#endif

#endif // SVC_BLE_INTERNAL_H
