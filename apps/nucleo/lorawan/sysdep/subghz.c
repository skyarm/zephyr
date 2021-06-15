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

#include "subghz.h"

#include <device.h>
#include <devicetree.h>
#include <drivers/gpio.h>
#include <stdio.h>

SUBGHZ_HandleTypeDef hsubghz;

/* SUBGHZ init function */
void MX_SUBGHZ_Init(void)
{
	hsubghz.Init.BaudratePrescaler = SUBGHZSPI_BAUDRATEPRESCALER_4;
	if (HAL_SUBGHZ_Init(&hsubghz) != HAL_OK) {
		assert(0);
	}
}

void HAL_SUBGHZ_MspInit(SUBGHZ_HandleTypeDef *subghzHandle)
{
	__HAL_RCC_SUBGHZSPI_CLK_ENABLE();
	IRQ_CONNECT(SUBGHZ_Radio_IRQn, 0, HAL_SUBGHZ_IRQHandler, &hsubghz, 0);
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

static const struct gpio_dt_spec rfswctrl1 =
	GPIO_DT_SPEC_GET(DT_ALIAS(rfswctrl1), gpios);
static const struct gpio_dt_spec rfswctrl2 =
	GPIO_DT_SPEC_GET(DT_ALIAS(rfswctrl2), gpios);
static const struct gpio_dt_spec rfswctrl3 =
	GPIO_DT_SPEC_GET(DT_ALIAS(rfswctrl3), gpios);
/*
   static const struct gpio_dt_spec rftcxovcc =
        GPIO_DT_SPEC_GET(DT_ALIAS(rftcxovcc), gpios);
 */
static const struct gpio_dt_spec dbradiorx =
	GPIO_DT_SPEC_GET(DT_ALIAS(dbradiorx), gpios);
static const struct gpio_dt_spec dbradiotx =
	GPIO_DT_SPEC_GET(DT_ALIAS(dbradiotx), gpios);

int32_t RBI_Init(void)
{
	rf_sw_ctrl1_gpio = rfswctrl1.port;
	gpio_pin_configure(rf_sw_ctrl1_gpio, rfswctrl1.pin, GPIO_OUTPUT_LOW);

	rf_sw_ctrl2_gpio = rfswctrl2.port;
	// gpio_pin_configure(rf_sw_ctrl2_gpio, RF_SW_CTRL2_PIN, GPIO_OUTPUT_LOW);

	rf_sw_ctrl3_gpio = rfswctrl3.port;
	// gpio_pin_configure(rf_sw_ctrl3_gpio, RF_SW_CTRL3_PIN, GPIO_OUTPUT_LOW);

	db_radio_rx_gpio = dbradiorx.port;
	// gpio_pin_configure(db_radio_rx_gpio, DB_RADIO_RX_PIN, GPIO_OUTPUT_LOW);

	db_radio_tx_gpio = dbradiotx.port;
	// gpio_pin_configure(db_radio_tx_gpio, DB_RADIO_TX_PIN, GPIO_OUTPUT_LOW);

	gpio_pin_set(rf_sw_ctrl1_gpio, rfswctrl1.pin, 0);
	gpio_pin_set(rf_sw_ctrl2_gpio, rfswctrl2.pin, 0);
	gpio_pin_set(rf_sw_ctrl3_gpio, rfswctrl3.pin, 0);

	return 0;
}

int32_t RBI_DeInit(void)
{
	/* Turn off switch */
	gpio_pin_set(rf_sw_ctrl1_gpio, rfswctrl1.pin, 0);
	gpio_pin_set(rf_sw_ctrl2_gpio, rfswctrl2.pin, 0);
	gpio_pin_set(rf_sw_ctrl3_gpio, rfswctrl3.pin, 0);

	return 0;
}

int32_t RBI_ConfigRFSwitch(RBI_Switch_TypeDef Config)
{
	switch (Config) {
	case RBI_SWITCH_OFF: {
		/* Turn off switch */
		gpio_pin_set(rf_sw_ctrl1_gpio, rfswctrl1.pin, 0);
		gpio_pin_set(rf_sw_ctrl2_gpio, rfswctrl2.pin, 0);
		gpio_pin_set(rf_sw_ctrl3_gpio, rfswctrl3.pin, 0);
		break;
	}
	case RBI_SWITCH_RX: {
		/*Turns On in Rx Mode the RF Switch */
		gpio_pin_set(rf_sw_ctrl1_gpio, rfswctrl1.pin, 1);
		gpio_pin_set(rf_sw_ctrl2_gpio, rfswctrl2.pin, 0);
		gpio_pin_set(rf_sw_ctrl3_gpio, rfswctrl3.pin, 1);
		break;
	}
	case RBI_SWITCH_RFO_LP: {
		/*Turns On in Tx Low Power the RF Switch */
		gpio_pin_set(rf_sw_ctrl1_gpio, rfswctrl1.pin, 1);
		gpio_pin_set(rf_sw_ctrl2_gpio, rfswctrl2.pin, 1);
		gpio_pin_set(rf_sw_ctrl3_gpio, rfswctrl3.pin, 1);
		break;
	}
	case RBI_SWITCH_RFO_HP: {
		/*Turns On in Tx High Power the RF Switch */
		gpio_pin_set(rf_sw_ctrl1_gpio, rfswctrl1.pin, 0);
		gpio_pin_set(rf_sw_ctrl2_gpio, rfswctrl2.pin, 1);
		gpio_pin_set(rf_sw_ctrl3_gpio, rfswctrl3.pin, 1);
		break;
	}
	default:
		break;
	}

	return 0;
}

void dbg_gpio_radio_rx(int v)
{
	gpio_pin_set(db_radio_rx_gpio, dbradiorx.pin, v);
}

void dbg_gpio_radio_tx(int v)
{
	gpio_pin_set(db_radio_tx_gpio, dbradiotx.pin, v);
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
