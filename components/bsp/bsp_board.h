/*
 * SPDX-FileCopyrightText: 2024 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: Unlicense OR CC0-1.0
 */
#ifndef BSP_BOARD_H
#define BSP_BOARD_H

/* Includes */
#include "sdkconfig.h"

#ifdef __cplusplus
extern "C" {
#endif

/* Types */
typedef struct {
    const char *device_name;
    int rgb_led_gpio;
} bsp_board_info_t;

/* Defines */
#define BSP_BOARD_DEVICE_NAME "NimBLE_GATT"
#define BSP_RGB_LED_GPIO CONFIG_BLINK_GPIO

/* Public variables */
extern const bsp_board_info_t g_bsp_board_info;

#ifdef __cplusplus
}
#endif

#endif // BSP_BOARD_H
