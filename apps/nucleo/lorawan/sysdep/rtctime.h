/*
 * Copyright (c) 2020 STMicroelectronics
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#ifndef __RTC_CLOCK_H__
#define __RTC_CLOCK_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <zephyr.h>

void RtcBkupWrite(uint32_t data0, uint32_t data1);

void RtcBkupRead(uint32_t *data0, uint32_t *data1);

void RtcStopAlarm(void);

void RtcSetAlarm(uint32_t timeout);

uint32_t RtcGetTimerContext(void);


static inline uint32_t RtcGetCalendarTime(uint16_t *msec)
{
	uint64_t ticks = k_uptime_get();

	*msec = (uint32_t)(ticks % 1000);
	return (uint32_t)(ticks / 1000);
}

static inline uint32_t RtcGetTimerValue(void)
{
	return k_uptime_get_32();
}

static inline uint32_t RtcGetMinimumTimeout(void)
{
	return 3;
}

static inline void DelayMsMcu(uint32_t ms)
{
	k_sleep(K_MSEC(ms));
}

static inline uint32_t RtcMs2Tick(uint32_t ms)
{
	return ms;
}

static inline uint32_t RtcTick2Ms(uint32_t ticks)
{
	return ticks;
}

#ifdef __cplusplus
}
#endif

#endif /* __RTC_TIMER_H__ */