/**
  ******************************************************************************
  * @file    subghz.h
  * @brief   This file contains all the function prototypes for
  *          the subghz.c file
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
#ifndef __SUBGHZ_H__
#define __SUBGHZ_H__

#ifdef __cplusplus
extern "C"
{
#endif

#include "stm32wlxx_hal.h"

  typedef enum
  {
    RBI_SWITCH_OFF = 0,
    RBI_SWITCH_RX = 1,
    RBI_SWITCH_RFO_LP = 2,
    RBI_SWITCH_RFO_HP = 3,
  } RBI_Switch_TypeDef;

#define RBI_CONF_RFO_LP_HP 0
#define RBI_CONF_RFO_LP 1
#define RBI_CONF_RFO_HP 2

/* Indicates the type of switch between the ones proposed by CONFIG Constants
 */
#define RBI_CONF_RFO RBI_CONF_RFO_LP_HP

  extern SUBGHZ_HandleTypeDef hsubghz;

  void MX_SUBGHZ_Init(void);
  
  /**
  * @brief  Init Radio Switch
  * @return BSP status
  */
  int32_t RBI_Init(void);

  /**
  * @brief  DeInit Radio Switch
  * @return BSP status
  */
  int32_t RBI_DeInit(void);

  /**
  * @brief  Configure Radio Switch.
  * @param  Config: Specifies the Radio RF switch path to be set.
  *         This parameter can be one of following parameters:
  *           @arg RADIO_SWITCH_OFF
  *           @arg RADIO_SWITCH_RX
  *           @arg RADIO_SWITCH_RFO_LP
  *           @arg RADIO_SWITCH_RFO_HP
  * @return BSP status
  */
  int32_t RBI_ConfigRFSwitch(RBI_Switch_TypeDef Config);

  /**
  * @brief  Return Board Configuration
  * @retval RBI_CONF_RFO_LP_HP
  * @retval RBI_CONF_RFO_LP
  * @retval RBI_CONF_RFO_HP
  */
  int32_t RBI_GetTxConfig(void);

  /**
  * @brief  Get Radio Wake Time
  * @return the wake upt time in ms
  */
  int32_t RBI_GetWakeUpTime(void);

  /**
  * @brief  Get If TCXO is to be present on board
  * @note   never remove called by MW,
  * @retval return 1 if present, 0 if not present
  */
  int32_t RBI_IsTCXO(void);

  /**
  * @brief  Get If DCDC is to be present on board
  * @note   never remove called by MW,
  * @retval return 1 if present, 0 if not present
  */
  int32_t RBI_IsDCDC(void);



#define RST RESET
/**
  * @brief Set RX TX pin to high or low level
  * GPIOB GPIO_PIN_12
  */
void dbg_gpio_radio_rx(int v);
void dbg_gpio_radio_tx(int v);

  /**
  * @brief Set RX pin to high or low level
  * GPIOB GPIO_PIN_12
  */
#define DBG_GPIO_RADIO_RX(set_rst) dbg_gpio_radio_rx(set_rst)

/**
  * @brief Set TX pin to high or low level
  * GPIOB GPIO_PIN_13
  */
#define DBG_GPIO_RADIO_TX(set_rst) dbg_gpio_radio_tx(set_rst)

#ifdef __cplusplus
}
#endif

#endif /* __SUBGHZ_H__ */
