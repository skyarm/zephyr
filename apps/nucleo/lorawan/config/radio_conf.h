/**
  ******************************************************************************
  * @file    radio_conf.h
  * @author  MCD Application Team
  * @brief   Header of Radio configuration
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

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __RADIO_CONF_H__
#define __RADIO_CONF_H__

#ifdef __cplusplus
extern "C"
{
#endif

/* Includes ------------------------------------------------------------------*/
#include <device.h>
#include <drivers/gpio.h>
#include <stdio.h>

#include <stm32_ll_utils.h>
#include "subghz.h"
#include "timer.h"
#include "utilities.h"
/**
  * @brief Max payload buffer size
  */
#define RADIO_RX_BUF_SIZE 255

/**
  * @brief drive value used anytime radio is NOT in TX low power mode
  */
#define SMPS_DRIVE_SETTING_DEFAULT SMPS_DRV_40

/**
  * @brief drive value used anytime radio is in TX low power mode
  *        TX low power mode is the worst case because the PA sinks from SMPS
  *        while in high power mode, current is sunk directly from the battery
  */
#define SMPS_DRIVE_SETTING_MAX SMPS_DRV_60

/**
  * @brief in XO mode, set internal capacitor (from 0x00 to 0x2F starting 11.2pF with 0.47pF steps)
  */
#define XTAL_DEFAULT_CAP_VALUE 0x20

/**
  * @brief Frequency error (in Hz) can be compensated here.
  *        warning XO frequency error generates (de)modulator sampling time error which can not be compensated
  */
#define RF_FREQUENCY_ERROR ((int32_t)0)

/**
  * @brief voltage of vdd tcxo.
  */
#define TCXO_CTRL_VOLTAGE TCXO_CTRL_1_7V

/* Function mapping */
/**
  * @brief SUBGHZ interface init to radio Middleware
  */
#define RADIO_INIT() MX_SUBGHZ_Init()

/**
  * @brief Delay interface to radio Middleware
  */
#define RADIO_DELAY_MS(ms) k_msleep(ms)


#define RADIO_MEMSET8(d, v, s) UTIL_MEM_set_8(d, v, s)

#ifdef __cplusplus
}
#endif

#endif /* __RADIO_CONF_H__*/