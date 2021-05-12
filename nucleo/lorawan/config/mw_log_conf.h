/**
  ******************************************************************************
  * @file    mw_log_conf.h
  * @author  MCD Application Team
  * @brief   Configure (enable/disable) traces for CM0
  *******************************************************************************
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
#ifndef __MW_LOG_CONF_H__
#define __MW_LOG_CONF_H__

#ifdef __cplusplus
extern "C" {
#endif

#define VLEVEL_OFF    0  /*!< used to set UTIL_ADV_TRACE_SetVerboseLevel() (not as message param) */
#define VLEVEL_ALWAYS 0  /*!< used as message params, if this level is given
                              trace will be printed even when UTIL_ADV_TRACE_SetVerboseLevel(OFF) */
#define VLEVEL_L 1       /*!< just essential traces */
#define VLEVEL_M 2       /*!< functional traces */
#define VLEVEL_H 3       /*!< all traces */

#define TS_OFF 0         /*!< Log without TimeStamp */
#define TS_ON 1          /*!< Log with TimeStamp */

#define T_REG_OFF  0     /*!< Log without bitmask */

#ifdef MW_LOG_ENABLED
#define MW_LOG(TS,VL, ...)   do{ {UTIL_ADV_TRACE_COND_FSend(VL, T_REG_OFF, TS, __VA_ARGS__);} }while(0)
#else  /* MW_LOG_ENABLED */
#define MW_LOG(TS,VL, ...)
#endif /* MW_LOG_ENABLED */

#ifdef __cplusplus
}
#endif

#endif /*__MW_LOG_CONF_H__ */