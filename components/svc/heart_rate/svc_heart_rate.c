/*
 * SPDX-FileCopyrightText: 2024 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: Unlicense OR CC0-1.0
 */
/* Includes */
#include "svc_heart_rate.h"

#include "esp_random.h"

/* Private variables */
static uint8_t s_svc_heart_rate_bpm;

/* Public functions */
void svc_heart_rate_update(void) {
    s_svc_heart_rate_bpm = 60 + (uint8_t)(esp_random() % 21);
}

uint8_t svc_heart_rate_get_bpm(void) { return s_svc_heart_rate_bpm; }
