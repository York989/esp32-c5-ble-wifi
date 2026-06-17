/*
 * SPDX-FileCopyrightText: 2024 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: Unlicense OR CC0-1.0
 */
#ifndef LED_H
#define LED_H

/* Includes */
/* STD APIs */
#include <stdbool.h>
#include <stdint.h>

/* ESP APIs */
#include "driver/gpio.h"
#include "esp_err.h"
#include "led_strip.h"
#include "sdkconfig.h"

/* Defines */
#define BLINK_GPIO CONFIG_BLINK_GPIO

/* Public function declarations */
uint8_t get_led_state(void);
bool led_is_rainbow_enabled(void);
esp_err_t led_set_rgb(uint8_t red, uint8_t green, uint8_t blue);
esp_err_t led_set_probe_gpio(int gpio_num);
int led_get_probe_gpio(void);
void led_on(void);
void led_off(void);
void led_init(void);

#endif // LED_H
