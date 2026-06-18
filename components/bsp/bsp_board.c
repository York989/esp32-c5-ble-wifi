/*
 * SPDX-FileCopyrightText: 2024 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: Unlicense OR CC0-1.0
 */
/* Includes */
#include "bsp_board.h"

/* Public variables */
const bsp_board_info_t g_bsp_board_info = {
    .device_name = BSP_BOARD_DEVICE_NAME,
    .rgb_led_gpio = BSP_RGB_LED_GPIO,
};
