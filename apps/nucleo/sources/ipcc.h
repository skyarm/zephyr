/*
 * Copyright (c) 2019 Jan Van Winkel <jan.van_winkel@dxplore.eu>
 *
 * Based on ST7789V sample:
 * Copyright (c) 2019 Marc Reilly
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#ifndef NUCLEO_IPCC_H
#define NUCLEO_IPCC_H

#ifdef __cplusplus
extern "C" {
#endif

void ipcc_rpt_join_request(const struct lorawan_node_join_callback_params *params);

void ipcc_rpt_data_sent(const struct lorawan_node_tx_callback_params *params);

void ipcc_rpt_data_received(uint8_t port, const void *data, uint8_t size,
			    const struct lorawan_node_rx_callback_params  *params)

void ipcc_rpt_class_changed(enum lorawan_node_class new_class);


#ifdef __cplusplus
}
#endif

#endif /* NUCLEO_IPCC_H */
