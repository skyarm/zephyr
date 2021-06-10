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
#include <zephyr.h>

#include "Commissioning.h"
#include "LmHandler.h"
#include "LmhpCompliance.h"
#include "RegionCN470.h"
#include "lora_info.h"
#include "rtclock.h"

static uint8_t battery_level_cb() { return 254; }
static uint16_t get_temperature_cb() { return 25; }

static void join_request_cb(LmHandlerJoinParams_t *params) {
  printk("Joined to Lorawan with status[%u], DataRate[%d].\n", params->Status,
         params->Datarate);
}

static void data_sent_cb(LmHandlerTxParams_t *params) {
  printk(
      "Sent [%d] bytes to Lorawan with status[%u], DataRate[%d], TxPower[%d], "
      "Channel[%d].\n",
      params->AppData.BufferSize, params->Status, params->Datarate,
      params->TxPower, params->Channel);
}

static void data_received_cb(LmHandlerAppData_t *appData,
                             LmHandlerRxParams_t *params) {
  printk(
      "Received [%u] bytes from Lorawan with status[%u], DataRate[%d], "
      "Rssi[%d], "
      "Snr[%d].\n",
      appData->BufferSize, params->Status, params->Datarate, params->Rssi,
      params->Snr);
}

K_SEM_DEFINE(sem_mac_process, 0, 1);
static void mac_process_cb(void) {
  k_sem_give(&sem_mac_process);
}

static void class_changed_cb(DeviceClass_t deviceClass) {
  printk("Class has been changed to CLASS %c.\n", "ABC"[deviceClass]);
}

static void beacon_status_changed_cb(const LmHandlerBeaconParams_t *param) {
  printk("Beacon state has been changed to %d.\n", param->State);
}

static LmHandlerCallbacks_t LmHandlerCallbacks = {
    .GetBatteryLevel = battery_level_cb,
    .GetTemperature = get_temperature_cb,
    .OnMacProcess = mac_process_cb,
    .OnJoinRequest = join_request_cb,
    .OnTxData = data_sent_cb,
    .OnRxData = data_received_cb,
    .OnClassChange = class_changed_cb,
    .OnBeaconStatusChange = beacon_status_changed_cb};

static LmHandlerParams_t LmHandlerParams = {
    .ActiveRegion = LORAMAC_REGION_CN470,
    .AdrEnable = LORAMAC_HANDLER_ADR_ON,
    .DefaultClass = CLASS_B,
    .TxDatarate = CN470_DEFAULT_DATARATE,
    .DutyCycleEnabled = false,
    .PingPeriodicity = 6};

static LmhpComplianceParams_t LmhpComplianceParams = {
    .AdrEnabled = true,
    .DutyCycleEnabled = false,
    .StopPeripherals = NULL,
    .StartPeripherals = NULL,
};

static const struct gpio_dt_spec led0 = GPIO_DT_SPEC_GET(DT_ALIAS(led0), gpios);

void main(void) {
  gpio_pin_configure(led0.port, led0.pin, GPIO_OUTPUT_LOW);
  gpio_pin_set(led0.port, led0.pin, 1);
  
  LoraInfo_Init();
  if (LmHandlerInit(&LmHandlerCallbacks) != LORAMAC_HANDLER_SUCCESS) {
    printk("LoRaMac wasn't properly initialized\n");
    assert(0);
  }

  if (LmHandlerConfigure(&LmHandlerParams) != LORAMAC_HANDLER_SUCCESS) {
    printk("LoRaMac wasn't properly initialized\n");
    assert(0);
  }

  // The LoRa-Alliance Compliance protocol package should always be
  // initialized and activated.
  LmHandlerPackageRegister(PACKAGE_ID_COMPLIANCE, &LmhpComplianceParams);
  // LmHandlerPackageRegister(PACKAGE_ID_CLOCK_SYNC, NULL);
  // LmHandlerPackageRegister(PACKAGE_ID_REMOTE_MCAST_SETUP, NULL);

  LmHandlerJoin(ACTIVATION_TYPE_OTAA);

  for (;;) {
    LmHandlerProcess();
    if (k_sem_take(&sem_mac_process, K_SECONDS(128)) == 0) {
      continue;
    }
  }
}
