#include <zephyr.h>

#include "timer.h"

static uint32_t rtc_time_base;
/* TODO: Use Non-volatile memory for backup */
static volatile uint32_t rtc_clock_base[2];

static void timer_callback(struct k_timer *_timer) {
  ARG_UNUSED(_timer);

  TimerIrqHandler();
}

K_TIMER_DEFINE(lora_timer, timer_callback, NULL);

void RtcBkupWrite(uint32_t data0, uint32_t data1) {
  rtc_clock_base[0] = data0;
  rtc_clock_base[1] = data1;
}

void RtcBkupRead(uint32_t *data0, uint32_t *data1) {
  *data0 = rtc_clock_base[0];
  *data1 = rtc_clock_base[1];
}

uint32_t RtcGetCalendarTime(uint16_t *milliseconds) {
  uint32_t now = k_uptime_get_32();

  *milliseconds = now;

  /* Return in seconds */
  return now / MSEC_PER_SEC;
}

uint32_t RtcGetTimerValue(void) { return k_uptime_get_32(); }

uint32_t RtcGetTimerElapsedTime(void) {
  return (k_uptime_get_32() - rtc_time_base);
}

uint32_t RtcGetMinimumTimeout(void) { return 1; }

void RtcStopAlarm(void) { k_timer_stop(&lora_timer); }

void RtcSetAlarm(uint32_t timeout) {
  k_timer_start(&lora_timer, K_MSEC(timeout), K_NO_WAIT);
}

uint32_t RtcSetTimerContext(void) {
  rtc_time_base = k_uptime_get_32();

  return rtc_time_base;
}

/* For us, 1 tick = 1 milli second. So no need to do any conversion here */
uint32_t RtcGetTimerContext(void) { return rtc_time_base; }

void DelayMsMcu(uint32_t ms) { k_sleep(K_MSEC(ms)); }

uint32_t RtcMs2Tick(uint32_t milliseconds) { return milliseconds; }

uint32_t RtcTick2Ms(uint32_t tick) { return tick; }
