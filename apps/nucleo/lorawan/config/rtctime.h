#ifndef __RTC_TIMER_H__
#define __RTC_TIMER_H__

#ifdef __cplusplus
extern "C"
{
#endif

#include <stdint.h>

void RtcBkupWrite(uint32_t data0, uint32_t data1);

void RtcBkupRead(uint32_t *data0, uint32_t *data1);


uint32_t RtcGetCalendarTime(uint16_t *milliseconds);


uint32_t RtcGetTimerValue(void);

uint32_t RtcGetTimerElapsedTime(void);

uint32_t RtcGetMinimumTimeout(void);

void RtcStopAlarm(void);

void RtcSetAlarm(uint32_t timeout);

uint32_t RtcSetTimerContext(void);

/* For us, 1 tick = 1 milli second. So no need to do any conversion here */
uint32_t RtcGetTimerContext(void);

void DelayMsMcu(uint32_t ms);


uint32_t RtcMs2Tick(uint32_t milliseconds);

uint32_t RtcTick2Ms(uint32_t tick);


#ifdef __cplusplus
}
#endif

#endif /* __RTC_TIMER_H__ */