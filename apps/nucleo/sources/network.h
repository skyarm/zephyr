/*
 * Copyright (c) 2019 Jan Van Winkel <jan.van_winkel@dxplore.eu>
 *
 * Based on ST7789V sample:
 * Copyright (c) 2019 Marc Reilly
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <stdbool.h>
#include <stdint.h>

#ifndef NECLEO_NETWORK_H
#define NECLEO_NETWORK_H

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @typedef ipm_send_t
 * @brief Callback API to send IPM messages
 *
 * See @a ipm_send() for argument definitions.
 */
bool network_init();

/**
 * @typedef ipm_send_t
 * @brief Callback API to send IPM messages
 *
 * See @a ipm_send() for argument definitions.
 */
int network_join();

/**
 * @typedef ipm_send_t
 * @brief Callback API to send IPM messages
 *
 * See @a ipm_send() for argument definitions.
 */
void network_process();

/**
 * @typedef ipm_send_t
 * @brief Callback API to send IPM messages
 *
 * See @a ipm_send() for argument definitions.
 */
int network_send(uint8_t port, uint8_t *data, size_t count);

void network_mac_process_callback();

uint8_t network_get_battery_level_callback();

uint16_t network_get_temperature_callback();

#ifdef __cplusplus
}
#endif

#endif
