/**
  ******************************************************************************
  * @file    subghz.c
  * @brief   This file provides code for the configuration
  *          of the SUBGHZ instances.
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2020 STMicroelectronics.
  * All rights reserved.</center></h2>
  *
  * This software component is licensed by ST under Ultimate Liberty license
  * SLA0044, the "License"; You may not use this file except in compliance with
  * the License. You may obtain a copy of the License at:
  *                             www.st.com/SLA0044
  *
  ******************************************************************************
  */

#include <device.h>
#include <drivers/gpio.h>
#include <stdio.h>
#include "subghz.h"

SUBGHZ_HandleTypeDef hsubghz;

/* SUBGHZ init function */
void MX_SUBGHZ_Init(void)
{
  hsubghz.Init.BaudratePrescaler = SUBGHZSPI_BAUDRATEPRESCALER_4;
  if (HAL_SUBGHZ_Init(&hsubghz) != HAL_OK)
  {
    assert(0);
  }
}

void HAL_SUBGHZ_MspInit(SUBGHZ_HandleTypeDef *subghzHandle)
{
  __HAL_RCC_SUBGHZSPI_CLK_ENABLE();

  IRQ_CONNECT(SUBGHZ_Radio_IRQn, 0,
              HAL_SUBGHZ_IRQHandler, &hsubghz, 0);
  irq_enable(SUBGHZ_Radio_IRQn);
}

void HAL_SUBGHZ_MspDeInit(SUBGHZ_HandleTypeDef *subghzHandle)
{
  __HAL_RCC_SUBGHZSPI_CLK_DISABLE();
  irq_disable(SUBGHZ_Radio_IRQn);
}

static const struct device *rf_sw_ctrl1_gpio;
static const struct device *rf_sw_ctrl2_gpio;
static const struct device *rf_sw_ctrl3_gpio;
/* FIXME
  static const struct device *rf_tcxo_vcc_gpio;
*/
static const struct device *db_radio_rx_gpio;
static const struct device *db_radio_tx_gpio;

#define RF_SW_CTRL1_NODE DT_ALIAS(rfswctrl1)
#define RF_SW_CTRL1_NAME DT_GPIO_LABEL(RF_SW_CTRL1_NODE, gpios)
#define RF_SW_CTRL1_PIN DT_GPIO_PIN(RF_SW_CTRL1_NODE, gpios)
#define RF_SW_CTRL1_FLAGS DT_GPIO_FLAGS(RF_SW_CTRL1_NODE, gpios)

#define RF_SW_CTRL2_NODE DT_ALIAS(rfswctrl2)
#define RF_SW_CTRL2_NAME DT_GPIO_LABEL(RF_SW_CTRL2_NODE, gpios)
#define RF_SW_CTRL2_PIN DT_GPIO_PIN(RF_SW_CTRL2_NODE, gpios)
#define RF_SW_CTRL2_FLAGS DT_GPIO_FLAGS(RF_SW_CTRL2_NODE, gpios)

#define RF_SW_CTRL3_NODE DT_ALIAS(rfswctrl3)
#define RF_SW_CTRL3_NAME DT_GPIO_LABEL(RF_SW_CTRL3_NODE, gpios)
#define RF_SW_CTRL3_PIN DT_GPIO_PIN(RF_SW_CTRL3_NODE, gpios)
#define RF_SW_CTRL3_FLAGS DT_GPIO_FLAGS(RF_SW_CTRL3_NODE, gpios)

#define RF_TCXO_VCC_NODE DT_ALIAS(rftxcovcc)
#define RF_TCXO_VCC_NAME DT_GPIO_LABEL(RF_TCXO_VCC_NODE, gpios)
#define RF_TCXO_VCC_PIN DT_GPIO_PIN(RF_TCXO_VCC_NODE, gpios)
#define RF_TCXO_VCC_FLAGS DT_GPIO_FLAGS(RF_TCXO_VCC_NODE, gpios)

#define DB_RADIO_RX_NODE DT_ALIAS(dbradiorx)
#define DB_RADIO_RX_NAME DT_GPIO_LABEL(DB_RADIO_RX_NODE, gpios)
#define DB_RADIO_RX_PIN DT_GPIO_PIN(DB_RADIO_RX_NODE, gpios)
#define DB_RADIO_RX_FLAGS DT_GPIO_FLAGS(DB_RADIO_RX_NODE, gpios)

#define DB_RADIO_TX_NODE DT_ALIAS(dbradiotx)
#define DB_RADIO_TX_NAME DT_GPIO_LABEL(DB_RADIO_TX_NODE, gpios)
#define DB_RADIO_TX_PIN DT_GPIO_PIN(DB_RADIO_TX_NODE, gpios)
#define DB_RADIO_TX_FLAGS DT_GPIO_FLAGS(DB_RADIO_TX_NODE, gpios)

int32_t RBI_Init(void)
{
  rf_sw_ctrl1_gpio = device_get_binding(RF_SW_CTRL1_NAME);
  gpio_pin_configure(rf_sw_ctrl1_gpio, RF_SW_CTRL1_PIN, GPIO_OUTPUT_LOW);

  rf_sw_ctrl2_gpio = device_get_binding(RF_SW_CTRL2_NAME);
  //gpio_pin_configure(rf_sw_ctrl2_gpio, RF_SW_CTRL2_PIN, GPIO_OUTPUT_LOW);

  rf_sw_ctrl3_gpio = device_get_binding(RF_SW_CTRL3_NAME);
  //gpio_pin_configure(rf_sw_ctrl3_gpio, RF_SW_CTRL3_PIN, GPIO_OUTPUT_LOW);

  db_radio_rx_gpio = device_get_binding(DB_RADIO_RX_NAME);
  //gpio_pin_configure(db_radio_rx_gpio, DB_RADIO_RX_PIN, GPIO_OUTPUT_LOW);

  db_radio_tx_gpio = device_get_binding(DB_RADIO_TX_NAME);
  //gpio_pin_configure(db_radio_tx_gpio, DB_RADIO_TX_PIN, GPIO_OUTPUT_LOW);

  gpio_pin_set(rf_sw_ctrl1_gpio, RF_SW_CTRL1_PIN, 0);
  gpio_pin_set(rf_sw_ctrl2_gpio, RF_SW_CTRL2_PIN, 0);
  gpio_pin_set(rf_sw_ctrl3_gpio, RF_SW_CTRL3_PIN, 0);

  return 0;
}

int32_t RBI_DeInit(void)
{
  /* Turn off switch */
  gpio_pin_set(rf_sw_ctrl1_gpio, RF_SW_CTRL1_PIN, 0);
  gpio_pin_set(rf_sw_ctrl2_gpio, RF_SW_CTRL2_PIN, 0);
  gpio_pin_set(rf_sw_ctrl3_gpio, RF_SW_CTRL3_PIN, 0);

  return 0;
}

int32_t RBI_ConfigRFSwitch(RBI_Switch_TypeDef Config)
{
  switch (Config)
  {
  case RBI_SWITCH_OFF:
  {
    /* Turn off switch */
    gpio_pin_set(rf_sw_ctrl1_gpio, RF_SW_CTRL1_PIN, 0);
    gpio_pin_set(rf_sw_ctrl2_gpio, RF_SW_CTRL2_PIN, 0);
    gpio_pin_set(rf_sw_ctrl3_gpio, RF_SW_CTRL3_PIN, 0);
    break;
  }
  case RBI_SWITCH_RX:
  {
    /*Turns On in Rx Mode the RF Switch */
    gpio_pin_set(rf_sw_ctrl1_gpio, RF_SW_CTRL1_PIN, 1);
    gpio_pin_set(rf_sw_ctrl2_gpio, RF_SW_CTRL2_PIN, 0);
    gpio_pin_set(rf_sw_ctrl3_gpio, RF_SW_CTRL3_PIN, 1);
    break;
  }
  case RBI_SWITCH_RFO_LP:
  {
    /*Turns On in Tx Low Power the RF Switch */
    gpio_pin_set(rf_sw_ctrl1_gpio, RF_SW_CTRL1_PIN, 1);
    gpio_pin_set(rf_sw_ctrl2_gpio, RF_SW_CTRL2_PIN, 1);
    gpio_pin_set(rf_sw_ctrl3_gpio, RF_SW_CTRL3_PIN, 1);
    break;
  }
  case RBI_SWITCH_RFO_HP:
  {
    /*Turns On in Tx High Power the RF Switch */
    gpio_pin_set(rf_sw_ctrl1_gpio, RF_SW_CTRL1_PIN, 0);
    gpio_pin_set(rf_sw_ctrl2_gpio, RF_SW_CTRL2_PIN, 1);
    gpio_pin_set(rf_sw_ctrl3_gpio, RF_SW_CTRL3_PIN, 1);
    break;
  }
  default:
    break;
  }

  return 0;
}

void dbg_gpio_radio_rx(int v)
{
  gpio_pin_set(db_radio_rx_gpio, DB_RADIO_RX_PIN, v);
}

void dbg_gpio_radio_tx(int v)
{
  gpio_pin_set(db_radio_tx_gpio, DB_RADIO_TX_PIN, v);
}

/* Radio maximum wakeup time (in ms) */
#define RF_WAKEUP_TIME 10U

/* Indicates whether or not TCXO is supported by the board
 * 0: TCXO not supported
 * 1: TCXO supported
 */
#define IS_TCXO_SUPPORTED 1U

/* Indicates whether or not DCDC is supported by the board
 * 0: DCDC not supported
 * 1: DCDC supported
 */
#define IS_DCDC_SUPPORTED 1U

int32_t RBI_GetTxConfig(void)
{
  return RBI_CONF_RFO;
}

int32_t RBI_GetWakeUpTime(void)
{
  return RF_WAKEUP_TIME;
}

int32_t RBI_IsTCXO(void)
{
  return IS_TCXO_SUPPORTED;
}

int32_t RBI_IsDCDC(void)
{
  return IS_DCDC_SUPPORTED;
}
