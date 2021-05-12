/*
 * Copyright (c) 2019 Jan Van Winkel <jan.van_winkel@dxplore.eu>
 *
 * Based on ST7789V sample:
 * Copyright (c) 2019 Marc Reilly
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <logging/log.h>
LOG_MODULE_REGISTER(sample, LOG_LEVEL_INF);

#include <device.h>
#include <drivers/gpio.h>
#include <math.h>
#include <stdio.h>
#include <sys/byteorder.h>
#include <zephyr.h>

#include "Commissioning.h"
#include "LmHandler.h"
#include "LmhpClockSync.h"
#include "LmhpCompliance.h"
#include "LmhpRemoteMcastSetup.h"
#include "RegionCN470.h"
#include "lora_info.h"

#define LED0_NODE DT_ALIAS(led0)
#define LED0_NAME DT_GPIO_LABEL(LED0_NODE, gpios)
#define LED0_PIN DT_GPIO_PIN(LED0_NODE, gpios)

static const struct device *led0 = NULL;
void sample_led() {
  led0 = device_get_binding(LED0_NAME);
  gpio_pin_configure(led0, LED0_PIN, GPIO_OUTPUT_LOW);
}

static uint8_t battery_level_cb() { return 255; }

static void join_request_cb(LmHandlerJoinParams_t *params) {
  LOG_INF("Joined to Lorawan with status[%u], DataRate[%d].", params->Status,
          params->Datarate);
  LmHandlerAppData_t app_data;
  app_data.Port = 1;
  app_data.Buffer = NULL;
  app_data.BufferSize = 0;
  LmHandlerSend(&app_data, LORAMAC_HANDLER_CONFIRMED_MSG, NULL, false);
}

static void data_sent_cb(LmHandlerTxParams_t *params) {
  LOG_INF(
      "Sent [%d] bytes to Lorawan with status[%u], DataRate[%d], TxPower[%d], "
      "Channel[%d].",
      params->AppData.BufferSize, params->Status, params->Datarate,
      params->TxPower, params->Channel);
}

static void data_received_cb(LmHandlerAppData_t *appData,
                             LmHandlerRxParams_t *params) {
  LOG_INF(
      "Received [%u] bytes from Lorawan with status[%u], DataRate[%d], Rssi[%d], "
      "Snr[%d].",
      appData->BufferSize, params->Status, params->Datarate, params->Rssi,
      params->Snr);
}

struct k_sem sem_process;
static void mac_process_cb(void) {
  k_sem_give(&sem_process); 
}

static void class_changed_cb(DeviceClass_t deviceClass) {
  LOG_INF("Class has been changed to CLASS %c.", "ABC"[deviceClass]);
}

static void beacon_status_changed_cb(const LmHandlerBeaconParams_t *param) {
  LOG_INF("Beacon state has been changed to %d.", param->State);
}

static LmHandlerCallbacks_t LmHandlerCallbacks = {
    .GetBatteryLevel = battery_level_cb,
    .GetTemperature = NULL,
    .OnMacProcess = mac_process_cb,
    .OnJoinRequest = join_request_cb,
    .OnTxData = data_sent_cb,
    .OnRxData = data_received_cb,
    .OnClassChange = class_changed_cb,
    .OnBeaconStatusChange = beacon_status_changed_cb
  };

static LmHandlerParams_t LmHandlerParams = {
    .ActiveRegion = LORAMAC_REGION_CN470,
    .AdrEnable = LORAMAC_HANDLER_ADR_ON,
    .DefaultClass = CLASS_B,
    .TxDatarate = CN470_DEFAULT_DATARATE,
    .DutyCycleEnabled = false,
    .PingPeriodicity = 4};

static LmhpComplianceParams_t LmhpComplianceParams = {
    .AdrEnabled = true,
    .DutyCycleEnabled = false,
    .StopPeripherals = NULL,
    .StartPeripherals = NULL,
};

void main(void) {
  k_sem_init(&sem_process, 0, 1);
  sample_led();

  LoraInfo_Init();
  if (LmHandlerInit(&LmHandlerCallbacks) != LORAMAC_HANDLER_SUCCESS) {
    LOG_ERR("LoRaMac wasn't properly initialized");
    assert(0);
  }

  if (LmHandlerConfigure(&LmHandlerParams) != LORAMAC_HANDLER_SUCCESS) {
    LOG_ERR("LoRaMac wasn't properly initialized");
    assert(0);
  }

  // The LoRa-Alliance Compliance protocol package should always be
  // initialized and activated.
  LmHandlerPackageRegister(PACKAGE_ID_COMPLIANCE, &LmhpComplianceParams);
  /*
    LmHandlerPackageRegister(PACKAGE_ID_CLOCK_SYNC, NULL);
    LmHandlerPackageRegister(PACKAGE_ID_REMOTE_MCAST_SETUP, NULL);
  */

  LmHandlerJoin(ACTIVATION_TYPE_OTAA);

  for (;;) {
    LmHandlerProcess();
    if (k_sem_take(&sem_process, K_SECONDS(30)) == -EAGAIN) {
      if (LmHandlerIsBusy()) {
        continue;
      }
      /*
      LoRaMacTxInfo_t txinfo;
      LmHandlerAppData_t app_data;
      if (LoRaMacQueryTxPossible(5, &txinfo) == LORAMAC_STATUS_OK) {
        app_data.Port = 1;
        app_data.Buffer = "hello";
        app_data.BufferSize = 5;
        LmHandlerSend(&app_data, LORAMAC_HANDLER_UNCONFIRMED_MSG, NULL, false);
      } else {
        app_data.Port = 1;
        app_data.Buffer = NULL;
        app_data.BufferSize = 0;
        LmHandlerSend(&app_data, LORAMAC_HANDLER_UNCONFIRMED_MSG, NULL, false);
      }
      */
    }
  }
}
