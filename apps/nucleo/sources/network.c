/*
 * Copyright (c) 2019 Jan Van Winkel <jan.van_winkel@dxplore.eu>
 *
 * Based on ST7789V sample:
 * Copyright (c) 2019 Marc Reilly
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <logging/log.h>
LOG_MODULE_DECLARE(nucleo, LOG_LEVEL_INF);

#include <stdbool.h>
#include <stdint.h>

#include "Commissioning.h"
#include "LmHandler.h"
#include "LmhpClockSync.h"
#include "LmhpCompliance.h"
#include "LmhpRemoteMcastSetup.h"
#include "RegionCN470.h"
#include "lora_info.h"

#include "network.h"

static void cb_class_changed(DeviceClass_t deviceClass);
static void cb_beacon_status_changed(const LmHandlerBeaconParams_t *param);
static void cb_data_received(LmHandlerAppData_t *appData,
                             LmHandlerRxParams_t *params);
static void cb_data_sent(LmHandlerTxParams_t *params);
static void cb_join_request(LmHandlerJoinParams_t *params);

static LmHandlerCallbacks_t LmHandlerCallbacks = {
    .GetBatteryLevel = network_get_battery_level_callback,
    .GetTemperature = network_get_temperature_callback,
    .OnMacProcess = network_mac_process_callback,
    .OnJoinRequest = cb_join_request,
    .OnTxData = cb_data_sent,
    .OnRxData = cb_data_received,
    .OnClassChange = cb_class_changed,
    .OnBeaconStatusChange = cb_beacon_status_changed
};

static LmHandlerParams_t LmHandlerParams = {
    .ActiveRegion = LORAMAC_REGION_CN470,
    .AdrEnable = LORAMAC_HANDLER_ADR_ON,
    .DefaultClass = CLASS_B,
    .TxDatarate = CN470_DEFAULT_DATARATE,
    .DutyCycleEnabled = false,
    .PingPeriodicity = 4
  };

static LmhpComplianceParams_t LmhpComplianceParams = {
    .AdrEnabled = true,
    .DutyCycleEnabled = false,
    .StopPeripherals = NULL,
    .StartPeripherals = NULL,
};

static void cb_join_request(LmHandlerJoinParams_t *params) {
  LOG_INF("Joined to Lorawan with status[%u], DataRate[%d].", params->Status,
          params->Datarate);
}

static void cb_data_sent(LmHandlerTxParams_t *params) {
  LOG_INF(
      "Sent [%d] bytes to Lorawan with status[%u], DataRate[%d], TxPower[%d], "
      "Channel[%d].",
      params->AppData.BufferSize, params->Status, params->Datarate,
      params->TxPower, params->Channel);
}

static void cb_data_received(LmHandlerAppData_t *appData,
                             LmHandlerRxParams_t *params) {
  LOG_INF(
      "Received [%u] bytes from Lorawan with status[%u], DataRate[%d], "
      "Rssi[%d], "
      "Snr[%d].",
      appData->BufferSize, params->Status, params->Datarate, params->Rssi,
      params->Snr);
}

static void cb_class_changed(DeviceClass_t deviceClass) {
  LOG_INF("Class has been changed to CLASS %c.", "ABC"[deviceClass]);
}

static void cb_beacon_status_changed(const LmHandlerBeaconParams_t *param) {
  LOG_INF("Beacon state has been changed to %d.", param->State);
}

bool network_init() {
  LoraInfo_Init();
  if (LmHandlerInit(&LmHandlerCallbacks) != LORAMAC_HANDLER_SUCCESS) {
    LOG_ERR("LoRaMac wasn't properly initialized");
    return false;
  }

  if (LmHandlerConfigure(&LmHandlerParams) != LORAMAC_HANDLER_SUCCESS) {
    LOG_ERR("LoRaMac wasn't properly initialized");
    return false;
  }

  // The LoRa-Alliance Compliance protocol package should always be
  // initialized and activated.
  LmHandlerPackageRegister(PACKAGE_ID_COMPLIANCE, &LmhpComplianceParams);
  /*
    LmHandlerPackageRegister(PACKAGE_ID_CLOCK_SYNC, NULL);
    LmHandlerPackageRegister(PACKAGE_ID_REMOTE_MCAST_SETUP, NULL);
  */
  return true;
}

int network_join() {
  uint32_t delay;
  LmHandlerJoin(LmHandlerParams.DefaultClass);
  LmHandlerGetJoinRx1Delay(&delay);
  return delay;
}

void network_process() {
  LmHandlerProcess();
}

int network_send(uint8_t port, uint8_t *data, size_t count) {
  LoRaMacTxInfo_t txinfo;
  if (LoRaMacQueryTxPossible(count, &txinfo) != LORAMAC_STATUS_OK) {
    return -1;
  }

  LmHandlerAppData_t app_data;
  app_data.Port = 1;
  app_data.Buffer = data;
  app_data.BufferSize = count;

  TimerTime_t next_time;
  LmHandlerErrorStatus_t rc =
      LmHandlerSend(&app_data, LORAMAC_HANDLER_CONFIRMED_MSG, &next_time, true);
  if (rc == LORAMAC_HANDLER_SUCCESS) {
    return next_time - TimerGetCurrentTime();
  } else if (rc == LORAMAC_HANDLER_NO_NETWORK_JOINED) {
    return 1;
  }
  LOG_ERR("LoRaMac send message failed, with return code[%d]", rc);
  return -1;
}


static volatile bool network_mac_process_requested = false; 

void network_mac_process_callback() {
    network_mac_process_requested = true;
}

uint8_t network_get_battery_level_callback() {
    //Always return max battery level
    return 255;
}

uint16_t network_get_temperature_callback() {
    //Always return 25
    return 25;
}

