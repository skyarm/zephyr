/*
 * Copyright (c) 2019 Jan Van Winkel <jan.van_winkel@dxplore.eu>
 *
 * Based on ST7789V sample:
 * Copyright (c) 2019 Marc Reilly
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#include <device.h>
#include <drivers/gpio.h>
#include <string.h>
#include <zephyr.h>

#include <lorawan_node.h>


static uint8_t cb_get_battery_level();
static uint16_t cb_get_temperature();
static void cb_join_request(const struct lorawan_node_cb_join_request_params *params);
static void cb_data_sent(const struct lorawan_node_cb_data_sent_params *params);
static void cb_data_received(uint8_t port, const void *data, uint8_t size,
			     const struct lorawan_node_cb_data_received_params  *params);
static void cb_mac_process(void);
static void cb_class_changed(enum lorawan_node_class new_class);
static void cb_beacon_status(const struct lorawan_node_cb_beacon_status_params *params);
static void cb_device_time(uint32_t seconds, uint16_t subseconds);

static const struct lorawan_node_config node_config = {
	.public_network = true,
	.repeater_supported = false,
	.active_region = LORAWAN_NODE_REGION_CN470,
	.network_id = 0,
	.adr_enabled = true,
	.tx_data_rate = LORAWAN_NODE_DR_0,
	.ping_periodicity = 6,
	.callbacks.get_battery_level = cb_get_battery_level,
	.callbacks.get_temperature = cb_get_temperature,
	.callbacks.mac_process = cb_mac_process,
	.callbacks.join_request = cb_join_request,
	.callbacks.data_sent = cb_data_sent,
	.callbacks.data_received = cb_data_received,
	.callbacks.class_changed = cb_class_changed,
	.callbacks.beacon_status = cb_beacon_status,
	.callbacks.device_time = cb_device_time
};

/**
 * @brief Get the current battery level
 * @retval value  Battery level ( 0: very low, 254: fully charged )
 */
static uint8_t cb_get_battery_level()
{
	return 254;
}

/**
 * @brief Get the current temperature
 * @retval value  Temperature in degree Celsius
 */
static uint16_t cb_get_temperature()
{
	return 25;
}

static void cb_join_request(const struct lorawan_node_cb_join_request_params *params)
{
	if (params->status == LORAWAN_NODE_EVENT_STATUS_OK) {
		lorawan_node_request_class(LORAWAN_NODE_CLASS_B);
	} else {
		lorawan_node_join(LORAWAN_NODE_ACTIVATION_OTAA);
	}
	printk("Joined to Lorawan with status[%u], DataRate[%d].\n", params->status,
	       params->data_rate);
}

static void cb_data_sent(const struct lorawan_node_cb_data_sent_params *params)
{
	printk("Sent to Lorawan with status[%u], DataRate[%d], TxPower[%d], Channel[%d].\n",
	       params->status, params->data_rate, params->tx_power,
	       params->channel);
}

static void cb_data_received(uint8_t port, const void *data, uint8_t size,
			     const struct lorawan_node_cb_data_received_params  *params)
{
	printk("Received [%u] bytes from Lorawan with status[%u], DataRate[%d], Rssi[%d], Snr[%d].\n",
	       size, params->status, params->data_rate, params->rssi, params->snr);
}

K_SEM_DEFINE(sem_mac_process, 0, 1);

static void cb_mac_process(void)
{
	k_sem_give(&sem_mac_process);
}

static void cb_class_changed(enum lorawan_node_class new_class)
{
	if (new_class == LORAWAN_NODE_CLASS_B) {
		char *p = "I'm on CLASS B";
		lorawan_node_send(1, (const uint8_t *)p, (uint8_t)strlen(p), false, true);
	}
	printk("Class has been changed to CLASS %c.\n", "ABC"[new_class]);
}

static void cb_beacon_status(const struct lorawan_node_cb_beacon_status_params *params)
{
	printk("Beacon state has been changed with status %d, GPS time[%u].\n", params->status,
	       params->time);
}

static void cb_device_time(uint32_t seconds, uint16_t subseconds)
{
}

void main(void)
{
	enum lorawan_node_status status = lorawan_node_init(&node_config);
	if (status != LORAWAN_NODE_STATUS_OK) {
		printk("lorawan_node_init failed, return code[%d]\n", status);
		assert(0);
	}

	status = lorawan_node_join(LORAWAN_NODE_ACTIVATION_OTAA);
	if (status != LORAWAN_NODE_STATUS_OK) {
		printk("lorawan_node_join failed, return code[%d]\n", status);
		assert(0);
	}

	for (;;) {
		lorawan_node_process();
		if (k_sem_take(&sem_mac_process, K_SECONDS(1000)) == 0) {
			continue;
		}
	}
}
