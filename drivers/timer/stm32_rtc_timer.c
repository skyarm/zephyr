/*
 * Copyright (c) 2019 Jan Van Winkel <jan.van_winkel@dxplore.eu>
 *
 * Based on ST7789V sample:
 * Copyright (c) 2019 Marc Reilly
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#include <device.h>
#include <zephyr.h>
#include <sys_clock.h>
#include <drivers/timer/system_timer.h>

#include <stm32_ll_pwr.h>
#include <stm32_ll_rcc.h>
#include <stm32_ll_rtc.h>

#define RTC_TIMER_PREDIV_S 12
#define RTC_TIMER_PREDIV_A ((1 << (15 - RTC_TIMER_PREDIV_S)) - 1)

#define RTC_TIMER_GUARD_VALUE 3

#define DT_DRV_COMPAT st_stm32_rtc

static struct k_spinlock rtc_timer_lock;

static volatile uint32_t rtc_timer_backup = UINT32_MAX;

static inline uint32_t rtc_timer_diff(uint32_t x, uint32_t y) {
  return x >= y ? x - y : y - x;
}

static void rtc_timer_alarm_handler(const struct device *unused) {
  ARG_UNUSED(unused);

  k_spinlock_key_t key = k_spin_lock(&rtc_timer_lock);
  if (LL_RTC_IsActiveFlag_ALRA(RTC) == 0) {
    k_spin_unlock(&rtc_timer_lock, key);
    return;
  }
  LL_RTC_ClearFlag_ALRA(RTC);
  LL_EXTI_ClearFlag_0_31(LL_EXTI_LINE_17);

  uint32_t current = LL_RTC_TIME_GetSubSecond(RTC);
  uint32_t ticks = rtc_timer_diff(rtc_timer_backup, current);
  rtc_timer_backup = current;

  k_spin_unlock(&rtc_timer_lock, key);

  sys_clock_announce(ticks);
}

static void rtc_timer_set_alarm(uint32_t ticks) {
  /* Disable RTC writing protection */
  LL_RTC_DisableWriteProtection(RTC);

  /* Disable alarm and interrupt, clear alarm a flag*/
  LL_RTC_ALMA_Disable(RTC);
  LL_RTC_DisableIT_ALRA(RTC);
  LL_RTC_ClearFlag_ALRA(RTC);

  /* Set alarm subsecond and mask */
  LL_RTC_WriteReg(RTC, ALRMASSR, RTC_ALARMSUBSECONDBINMASK_NONE);
  LL_RTC_ALMA_SetSubSecond(RTC, ticks);

  /* Enable alarm and interrupt again */
  LL_RTC_ALMA_Enable(RTC);
  LL_RTC_EnableIT_ALRA(RTC);

  /* Enable RTC writing protection */
  LL_RTC_EnableWriteProtection(RTC);

  /* 
    Enable EXTI interrupt
    Let me wake up from power saving mode 
  */
  LL_EXTI_EnableIT_0_31(LL_EXTI_LINE_17);
}

int sys_clock_driver_init(const struct device *dev) {
  ARG_UNUSED(dev);
#if defined(LL_APB1_GRP1_PERIPH_PWR)
  /* Enable the power interface clock */
  LL_APB1_GRP1_EnableClock(LL_APB1_GRP1_PERIPH_PWR);
#endif /* LL_APB1_GRP1_PERIPH_PWR */
  /* Enable backup domain */
  LL_PWR_EnableBkUpAccess();
  while (LL_PWR_IsEnabledBkUpAccess() == 0) {
    /* Wait for*/
  }
#ifdef CONFIG_STM32_RTC_CLOCK_LSE
  if (LL_RCC_GetRTCClockSource() != LL_RCC_RTC_CLKSOURCE_LSE) {
#else 
  if (LL_RCC_GetRTCClockSource() != LL_RCC_RTC_CLKSOURCE_LSI) {
#endif
    uint32_t reg = READ_BIT(RCC->BDCR, ~(RCC_BDCR_RTCSEL));
    LL_RCC_ForceBackupDomainReset();
    LL_RCC_ReleaseBackupDomainReset();
    RCC->BDCR = reg;
  }
#ifdef CONFIG_STM32_RTC_CLOCK_LSE
  while (LL_RCC_LSE_IsReady() == 0) {
#else 
  while (LL_RCC_LSI_IsReady() == 0) {
#endif
  }

#ifdef CONFIG_STM32_RTC_TIMER_CLOCK_LSE
  LL_RCC_SetRTCClockSource(LL_RCC_RTC_CLKSOURCE_LSE);
#else 
  LL_RCC_SetRTCClockSource(LL_RCC_RTC_CLKSOURCE_LSI);
#endif

  LL_RCC_EnableRTC();
  LL_APB1_GRP1_EnableClock(LL_APB1_GRP1_PERIPH_RTCAPB);

  LL_RTC_DisableWriteProtection(RTC);
  if (LL_RTC_EnterInitMode(RTC) == 0) {
    assert(0);
  }

  LL_RTC_SetHourFormat(RTC, LL_RTC_HOURFORMAT_24HOUR);
  LL_RTC_SetAlarmOutEvent(RTC, LL_RTC_ALARMOUT_DISABLE);
  LL_RTC_SetOutputPolarity(RTC, LL_RTC_OUTPUTPOLARITY_PIN_HIGH);
  LL_RTC_DisableTamperOutput(RTC);

  LL_RTC_WriteReg(RTC, PRER, (RTC_TIMER_PREDIV_A << RTC_PRER_PREDIV_A_Pos));
  /* Configure the Binary mode */
  LL_RTC_SetBinaryMode(RTC, RTC_BINARY_ONLY);

  LL_RTC_ExitInitMode(RTC);

  LL_RTC_DisableAlarmPullUp(RTC);
  LL_RTC_SetAlarmOutputType(RTC, RTC_OUTPUT_TYPE_OPENDRAIN);
  LL_RTC_DisableOutput2(RTC);


  LL_RTC_EnableShadowRegBypass(RTC);

  LL_RTC_EnableWriteProtection(RTC);

  LL_RTC_ClearFlag_ALRA(RTC);

  IRQ_CONNECT(DT_IRQN(DT_NODELABEL(rtc)), DT_IRQ(DT_NODELABEL(rtc), priority),
              rtc_timer_alarm_handler, NULL, 0);
  irq_enable(DT_IRQN(DT_NODELABEL(rtc)));

  return 0;
}

void sys_clock_set_timeout(int32_t ticks, bool idle) {
  ARG_UNUSED(idle);

  if (IS_ENABLED(CONFIG_TICKLESS_KERNEL) == 0) {
    return;
  }

  k_spinlock_key_t key = k_spin_lock(&rtc_timer_lock);

  uint32_t timeout, current;
  current = LL_RTC_TIME_GetSubSecond(RTC);
  timeout = current - (uint32_t)ticks;
  /* passing ticks==1 means "announce the next tick",
   * ticks value of zero (or even negative) is legal and
   * treated identically: it simply indicates the kernel would like the
   * next tick announcement as soon as possible.
   */
  if (LL_RTC_IsActiveFlag_ALRA(RTC) != 0) {
    if (rtc_timer_diff(rtc_timer_backup, current) < RTC_TIMER_GUARD_VALUE) {
      /* interrupt happens or happens soon.
       * It's impossible to set autoreload value. */
      k_spin_unlock(&rtc_timer_lock, key);
      return;
    }
  }
  if ((uint32_t)ticks < RTC_TIMER_GUARD_VALUE) {
    timeout = current - RTC_TIMER_GUARD_VALUE;
  }
  rtc_timer_set_alarm(timeout);

  k_spin_unlock(&rtc_timer_lock, key);
}

uint32_t sys_clock_elapsed(void) {
  if (IS_ENABLED(CONFIG_TICKLESS_KERNEL) == 0) {
    return 0;
  }
  k_spinlock_key_t key = k_spin_lock(&rtc_timer_lock);
  uint32_t ticks =
      rtc_timer_diff(rtc_timer_backup, LL_RTC_TIME_GetSubSecond(RTC));
  k_spin_unlock(&rtc_timer_lock, key);
  return ticks;
}

uint32_t sys_clock_cycle_get_32(void) {
  k_spinlock_key_t key = k_spin_lock(&rtc_timer_lock);
  uint64_t ticks = UINT32_MAX - LL_RTC_TIME_GetSubSecond(RTC);
  k_spin_unlock(&rtc_timer_lock, key);
  return ticks;
}
