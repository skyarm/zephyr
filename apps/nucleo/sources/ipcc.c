/*
 * Copyright (c) 2019 Jan Van Winkel <jan.van_winkel@dxplore.eu>
 *
 * Based on ST7789V sample:
 * Copyright (c) 2019 Marc Reilly
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <string.h>
#include <drivers/ipm.h>

#include <lorawan_node.h>

#define IPM_CHANNEL_ID 0
static const struct device *ipm_device = NULL;
static uint8_t ipm_buffer[255];

enum ipm_reports {
	ipm_rpt_core_started,
	ipm_rpt_join_request,
	ipm_rpt_data_sent,
	ipm_rpt_sent_acked,
	ipm_rpt_data_received,
	ipm_rpt_class_changed,
	ipm_rpt_get_datetime,
	ipm_rpt_current_class
};

enum ipm_command {
	ipm_cmd_get_datetime,
	ipm_cmd_send_message,
	ipm_cmd_change_class,
	ipm_cmd_current_class
};

struct ipm_message {
	uint8_t port;
	uint8_t confirmed;
	uint8_t size;
	uint8_t data[255];
};

struct ipm_request {
	bool get_datetime;
	bool change_class;
	bool send_message;
	bool current_class;
	union {
		uint8_t new_class;
		struct ipm_message message;
	};
};

void ipcc_rpt_join_request(const struct lorawan_node_join_callback_params *params)
{
    uint8_t status = params->status == LORAWAN_NODE_EVENT_STATUS_OK ? 0 : 1;

	ipm_buffer[0] = (uint8_t)ipm_rpt_join_request;
	ipm_buffer[1] = status;
	ipm_send(ipm_device, 0, IPM_CHANNEL_ID, ipm_buffer, 2);
}

void ipcc_rpt_data_sent(const struct lorawan_node_cb_data_sent_params *params)
{	
}

void ipcc_rpt_data_received(uint8_t port, const void *data, uint8_t size,
			    const struct lorawan_node_cb_data_received_params  *params)
{
	if (params->status == LORAWAN_NODE_EVENT_STATUS_OK) {
		ipm_buffer[0] = (uint8_t)ipm_rpt_data_received;
		ipm_buffer[1] = port;
		ipm_buffer[2] = size;
		memcpy(ipm_buffer + 3, data, size);
		ipm_send(ipm_device, 0, IPM_CHANNEL_ID, ipm_buffer, size + 3);
	}
}

void ipcc_rpt_class_changed(enum lorawan_node_class new_class) 
{	
	ipm_buffer[0] = (uint8_t)ipm_rpt_class_changed;
	ipm_buffer[1] = new_class;
	ipm_send(ipm_device, 0, IPM_CHANNEL_ID, ipm_buffer, 2);
}

static void cb_ipm(const struct device *device, void *user_data,
		   uint32_t id, volatile void *data)
{
	if (id != 0 || lorawan_node_is_initialed() == false) {
		return;
	}
	uint8_t *p = (uint8_t *)data;

	switch (p[0]) {
	case ipm_cmd_get_datetime:
	{
		ipm_request.get_datetime = true;
		k_sem_give(&sem_mac_process);
	}
	break;
	case ipm_cmd_send_message:
	{
		ipm_request.message.port = p[1];
		ipm_request.message.confirmed = p[2];
		ipm_request.message.size = p[3];
		memcpy(ipm_request.message.data, p + 4,  ipm_request.message.size);
		ipm_request.send_message = true;
		k_sem_give(&sem_mac_process);
	}
	break;
	case ipm_cmd_change_class:
	{
		ipm_request.new_class = p[1];
		ipm_request.change_class = true;
		k_sem_give(&sem_mac_process);
	}
	break;
	case ipm_cmd_current_class:
	{
		ipm_request.current_class = true;
		k_sem_give(&sem_mac_process);
	}
	break;
	default:
	{
		printk("Invalid IPM command[%d\n", p[0]);
	}
	break;
	}
}

static bool cmd_change_class()
{
	enum lorawan_node_class current_class = lorawan_node_get_current_class();

	if (current_class == ipm_request.new_class) {
		ipm_request.change_class = false;
		return true;
	}

	if (lorawan_node_is_busy() || lorawan_node_classb_pending()) {
		/* device is busy or not joined or is switching to Class B*/
		return false;
	}

	if (current_class == LORAWAN_NODE_CLASS_A) {
		/* Class A can change to other class immediately */
		lorawan_node_request_class(ipm_request.new_class);
		ipm_request.change_class = false;
		return true;
	} else {
		/* change to Class A first, next loop we can change to CLass B or C */
		lorawan_node_request_class(LORAWAN_NODE_CLASS_A);
		/* FIXME: need to notify the NS server ?*/
		lorawan_node_request_class(ipm_request.new_class);
		ipm_request.change_class = false;
		return true;
	}
}

static bool cmd_current_class()
{
	ipm_buffer[0] = ipm_rpt_current_class;
	if (lorawan_node_is_busy()) {
		/* device is busy or not joined or is switching to Class B*/
		ipm_buffer[1] = 0xFF;
		ipm_send(ipm_device, 0, IPM_CHANNEL_ID, ipm_buffer, 2);
		ipm_request.current_class = false;
		return true;
	}

	if (lorawan_node_classb_pending()) {
		ipm_buffer[1] = 3;
		ipm_send(ipm_device, 0, IPM_CHANNEL_ID, ipm_buffer, 2);
		ipm_request.current_class = false;
		return true;
	}

	enum lorawan_node_class current_class = lorawan_node_get_current_class();
	ipm_buffer[1] = current_class;
	ipm_send(ipm_device, 0, IPM_CHANNEL_ID, ipm_buffer, 2);
	ipm_request.current_class = false;
	
	return true;
}

static bool cmd_send_message()
{
	if (lorawan_node_is_busy() || lorawan_node_classb_pending()) {
		/* device is busy or not joined or is switching to Class B*/
		return false;
	}
	lorawan_node_send(ipm_request.message.port, ipm_request.message.data,
			  ipm_request.message.size, ipm_request.message.confirmed, true);
	ipm_request.send_message = false;
	return true;
}

static bool cmd_get_datetime()
{
	enum lorawan_node_class current_class = lorawan_node_get_current_class();

	if (current_class == LORAWAN_NODE_CLASS_B) {
		/* return the datetime immediately */
		uint16_t subseconds;
		uint32_t seconds = lorawan_node_get_current_time(&subseconds);

		ipm_buffer[0] = ipm_rpt_get_datetime;
		*((uint32_t *)(ipm_buffer + 1)) = seconds;
		*((uint16_t *)(ipm_buffer + 5)) = subseconds;
		ipm_send(ipm_device, 0, IPM_CHANNEL_ID, ipm_buffer, 7);

		ipm_request.get_datetime = false;
		return true;
	}

	if (lorawan_node_is_busy() || lorawan_node_classb_pending()) {
		/* device is busy or not joined or is switching to Class B*/
		return false;
	}

	lorawan_node_device_time_req();
	ipm_request.get_datetime = false;
	return true;
}

ipm_device = DEVICE_DT_GET_ANY(st_stm32_ipcc_mailbox);
	ipm_register_callback(ipm_device, cb_ipm, NULL);



