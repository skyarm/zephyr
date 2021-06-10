#include <zephyr.h>

#include "timer.h"
#include "rtclock.h"

static volatile uint32_t rtc_clock_context;
static uint32_t rtc_clock_backup[2];

static void rtc_clock_callback(struct k_timer *timer) {
  ARG_UNUSED(timer);
  TimerIrqHandler();
}

K_TIMER_DEFINE(rtc_clock_timer, rtc_clock_callback, NULL);

void RtcStopAlarm(void) {
  k_timer_stop(&rtc_clock_timer); 
}

void RtcSetAlarm(uint32_t timeout) {
  k_timer_start(&rtc_clock_timer, K_MSEC(timeout), K_NO_WAIT);
}

void RtcBkupWrite(uint32_t second, uint32_t subsecond) {
  rtc_clock_backup[0] = second;
  rtc_clock_backup[1] = subsecond;
}

void RtcBkupRead(uint32_t *second, uint32_t *subsecond) {
  *second = rtc_clock_backup[0];
  *subsecond = rtc_clock_backup[1];
}

uint32_t RtcGetTimerElapsedTime(void) {
  return RtcGetTimerValue() - rtc_clock_context;
}

uint32_t RtcSetTimerContext(void) {
  rtc_clock_context = RtcGetTimerValue();
  return rtc_clock_context;
}

/* For us, 1 tick = 1 milli second. So no need to do any conversion here */
uint32_t RtcGetTimerContext(void) { 
  return rtc_clock_context; 
}

