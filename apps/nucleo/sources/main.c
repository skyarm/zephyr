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

#include "Commissioning.h"
#include "LmHandler.h"
#include "LmhpCompliance.h"
#include "RegionCN470.h"
#include "lora_info.h"

static uint8_t get_battery_level_cb()
{
	return 254;
}
static uint16_t get_temperature_cb()
{
	return 25;
}

static void join_request_cb(LmHandlerJoinParams_t *params)
{
	printk("Joined to Lorawan with status[%u], DataRate[%d].\n", params->Status,
	       params->Datarate);
}

static void data_sent_cb(LmHandlerTxParams_t *params)
{
	printk("Sent [%d] bytes to Lorawan with status[%u], DataRate[%d], TxPower[%d], Channel[%d].\n",
	       params->AppData.BufferSize, params->Status, params->Datarate, params->TxPower,
	       params->Channel);
}

static void data_received_cb(LmHandlerAppData_t *appData,
			     LmHandlerRxParams_t *params)
{
	printk("Received [%u] bytes from Lorawan with status[%u], DataRate[%d], Rssi[%d], Snr[%d].\n",
	       appData->BufferSize, params->Status, params->Datarate, params->Rssi,
	       params->Snr);
}

K_SEM_DEFINE(sem_mac_process, 0, 1);

static void mac_process_cb(void)
{
	k_sem_give(&sem_mac_process);
}

static void class_changed_cb(DeviceClass_t deviceClass)
{
	if (deviceClass == CLASS_B) {
		char *p = "I'm on CLASS B";
		LmHandlerAppData_t appData = {
			.Buffer = (uint8_t *)p, .BufferSize = (uint8_t)strlen(p), .Port = 1
		};
		LmHandlerSend(&appData, LORAMAC_HANDLER_UNCONFIRMED_MSG, NULL, true);
	}
	printk("Class has been changed to CLASS %c.\n", "ABC"[deviceClass]);
}

static void beacon_status_changed_cb(const LmHandlerBeaconParams_t *param)
{
	printk("Beacon state has been changed to %d.\n", param->State);
}

static const LmHandlerCallbacks_t LmHandlerCallbacks = {
	.GetBatteryLevel = get_battery_level_cb,
	.GetTemperature = get_temperature_cb,
	.OnMacProcess = mac_process_cb,
	.OnJoinRequest = join_request_cb,
	.OnTxData = data_sent_cb,
	.OnRxData = data_received_cb,
	.OnClassChange = class_changed_cb,
	.OnBeaconStatusChange = beacon_status_changed_cb
};

static const LmHandlerParams_t LmHandlerParams = {
	.ActiveRegion = LORAMAC_REGION_CN470,
	.AdrEnable = LORAMAC_HANDLER_ADR_ON,
	.DefaultClass = CLASS_B,
	.TxDatarate = CN470_DEFAULT_DATARATE,
	.DutyCycleEnabled = false,
	.PingPeriodicity = 6
};

void main(void)
{
	LoraInfo_Init();
	if (LmHandlerInit(&LmHandlerCallbacks) != LORAMAC_HANDLER_SUCCESS) {
		printk("LoRaMac wasn't properly initialized\n");
		assert(0);
	}

	if (LmHandlerConfigure(&LmHandlerParams) != LORAMAC_HANDLER_SUCCESS) {
		printk("LoRaMac wasn't properly initialized\n");
		assert(0);
	}

	LmHandlerJoin(ACTIVATION_TYPE_OTAA);

	for (;;) {
		LmHandlerProcess();
		if (k_sem_take(&sem_mac_process, K_SECONDS(1000)) == 0) {
			continue;
		}
	}
}
