/*
 * Copyright (c) 2020 STMicroelectronics
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <zephyr.h>
#include "rtctime.h"

static uint32_t rtc_clock_backup[2];

extern void TimerIrqHandler(void);

static void rtc_clock_callback(struct k_timer *timer)
{
	ARG_UNUSED(timer);
	TimerIrqHandler();
}

K_TIMER_DEFINE(rtc_clock_timer, rtc_clock_callback, NULL);

void RtcStopAlarm(void)
{
	k_timer_stop(&rtc_clock_timer);
}

void RtcSetAlarm(uint32_t timeout)
{
	k_timer_start(&rtc_clock_timer, K_MSEC(timeout), K_NO_WAIT);
}

void RtcBkupWrite(uint32_t second, uint32_t subsecond)
{
	rtc_clock_backup[0] = second;
	rtc_clock_backup[1] = subsecond;
}

void RtcBkupRead(uint32_t *second, uint32_t *subsecond)
{
	*second = rtc_clock_backup[0];
	*subsecond = rtc_clock_backup[1];
}
