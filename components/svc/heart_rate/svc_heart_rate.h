/*
 * SPDX-FileCopyrightText: 2024 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: Unlicense OR CC0-1.0
 */
#ifndef SVC_HEART_RATE_H
#define SVC_HEART_RATE_H

/* Includes */
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Update the mock heart-rate value.
 */
void svc_heart_rate_update(void);

/**
 * @brief Get the latest mock heart-rate value in BPM.
 *
 * @return Heart-rate value in beats per minute.
 */
uint8_t svc_heart_rate_get_bpm(void);

#ifdef __cplusplus
}
#endif

#endif // SVC_HEART_RATE_H
