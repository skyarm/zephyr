/**
 * Copyright (c) 2019 STMicroelectronics
 * Copyright (c) 2021 Skyarm Technologies
 * SPDX-License-Identifier: Apache-2.0
 */
#include <device.h>
#include <drivers/clock_control.h>
#include <drivers/clock_control/stm32_clock_control.h>
#include <drivers/timer/system_timer.h>
#include <stm32_ll_pwr.h>
#include <stm32_ll_rcc.h>
#include <stm32_ll_rtc.h>
#include <sys_clock.h>
#include <zephyr.h>

/**
 * This module implements a kernel device driver for the accurate
 * and low power waking up system clock timer.
 *
 * The RTC need Power Backup Access be Enabled, But In
 * /drivers/clock_control/clock_stm32l4_l5_wb_wl.c The Power
 * Backup Access is disabled by call LL_PWR_DisableBkUpAccess(),
 * So The LL_PWR_EnableBkUpAccess() must be called after reconfigure
 * the rcc clocks when wake up from PM module,
 * In your soc/arm/st_stm32/stm32xx/power.c, modify it like this:
 *    stm32_clock_control_init(NULL);
 *  + LL_PWR_EnableBkUpAccess();
 *  + while (LL_PWR_IsEnabledBkUpAccess() == 0) {
 *  + };
 *
 * The CONFIG_SYS_CLOCK_TICKS_PER_SEC is configured to 1024 by
 * default and recommending when configure the LSE as RTC clock.
 * But also can be configured to 2048 or 4096 or 8912 and more, thus:
 * max timer value is about 48 days if it is configured to 1024,
 * max timer value is about 24 days if it is configured to 2048,
 * and so on.
 *
 * The CONFIG_SYS_CLOCK_TICKS_PER_SEC is configured to 1000 by
 * default and recommending, when configure the LSI as RTC clock.
 * The max timer value close to LSE.
 *
 * When CONFIG_SYS_CLOCK_TICKS_PER_SEC is configured to 1024, The MCU
 * can be in STOP0, STOP1,STOP2 mode up to 48 days when in PM model.
 **/

/**
 * Modify soc/st_stm32/common/Kconfig.defconfig.series like this
 *   config CORTEX_M_SYSTICK
 *      default n if STM32_RTC_TIMER
 *   config SYS_CLOCK_TICKS_PER_SEC
 *      default 1024 if STM32_RTC_TIMER && STM32_RTC_CLOCK_LSE
 *      default 1000 if STM32_RTC_TIMER && STM32_RTC_CLOCK_LSI
 * Modify drivers/timer/Kconfig, like this
 *   source "drivers/timer/Kconfig.stm32_rtc"
 * Modify soc/arm/st_stm32/stm32xx/Kconfig.defconfig.series
 *    if PM
 *        config PM_DEVICE
 *            default y
 *        config STM32_RTC_TIMER
 *            default y
 *    endif # PM
 **/

#if defined(CONFIG_SOC_SERIES_STM32WBX) ||     \
	defined(CONFIG_SOC_SERIES_STM32WLX) || \
	(defined(CONFIG_SOC_SERIES_STM32L5X) && !defined(__ARM_FEATURE_CMSE))
#define RTC_TIMER_EXTI_LINE LL_EXTI_LINE_17
#elif defined(CONFIG_SOC_SERIES_STM32L4X) ||				       \
	(defined(CONFIG_SOC_SERIES_STM32L5X) && defined(__ARM_FEATURE_CMSE) && \
	(__ARM_FEATURE_CMSE == 3U))
#define RTC_TIMER_EXTI_LINE LL_EXTI_LINE_18
#else
#error "Unexpected SOC series to configure RTC_TIMER_EXTI_LINE"
#endif

#define RTC_TIMER_MINIMUM_VALUE 3

#define DT_DRV_COMPAT st_stm32_rtc

#define RTC_TIMER_PREDIV_A \
	((DT_INST_PROP(0, prescaler) / CONFIG_SYS_CLOCK_TICKS_PER_SEC) - 1)

static struct k_spinlock rtc_timer_lock;

static volatile uint32_t rtc_timer_backup = UINT32_MAX;

static inline uint32_t rtc_timer_diff(uint32_t x, uint32_t y)
{
	return x >= y ? x - y : y - x;
}

static void rtc_timer_alarm_isr(const struct device *unused)
{
	ARG_UNUSED(unused);

	k_spinlock_key_t key = k_spin_lock(&rtc_timer_lock);

	if (LL_RTC_IsActiveFlag_ALRA(RTC) == 0) {
		k_spin_unlock(&rtc_timer_lock, key);
		return;
	}
	LL_RTC_ClearFlag_ALRA(RTC);
	LL_EXTI_ClearFlag_0_31(RTC_TIMER_EXTI_LINE);

	/* Save the current subsecond and return elapsed subseconds
	   since last rtc_timer_alarm_isr be called */
	uint32_t current = LL_RTC_TIME_GetSubSecond(RTC);
	uint32_t elapsed = rtc_timer_diff(rtc_timer_backup, current);

	rtc_timer_backup = current;

	k_spin_unlock(&rtc_timer_lock, key);

	sys_clock_announce(elapsed);
}

static void rtc_timer_set_alarm(uint32_t ticks)
{
	/* Disable RTC writing protection */
	LL_RTC_DisableWriteProtection(RTC);

	/* Disable alarm and IT, clear alarm flag*/
	LL_RTC_ALMA_Disable(RTC);
	LL_RTC_DisableIT_ALRA(RTC);
	LL_RTC_ClearFlag_ALRA(RTC);

	/* Set alarm subsecond and mask */
	LL_RTC_WriteReg(RTC, ALRMASSR, RTC_ALARMSUBSECONDBINMASK_NONE);
	LL_RTC_ALMA_SetSubSecond(RTC, ticks);

	/* Enable alarm and interrupt again */
	LL_RTC_ALMA_Enable(RTC);
	LL_RTC_EnableIT_ALRA(RTC);

	/* Enable RTC writing protection again */
	LL_RTC_EnableWriteProtection(RTC);

	/* Enable EXTI IT, Let me wake up from low power mode */
	LL_EXTI_EnableIT_0_31(RTC_TIMER_EXTI_LINE);
}

static const struct stm32_pclken rtc_clock_pclken = {
	.enr = DT_INST_CLOCKS_CELL(0, bits), .bus = DT_INST_CLOCKS_CELL(0, bus)
};

int sys_clock_driver_init(const struct device *dev)
{
	ARG_UNUSED(dev);

	/* Enable backup domain and it must be enabled always,
	   can't disable it, Because RTC need it all the time. */
	LL_PWR_EnableBkUpAccess();
	while (LL_PWR_IsEnabledBkUpAccess() == false) {
		/* Wait for backup domain to be accessable */
	}
#ifdef CONFIG_STM32_RTC_CLOCK_LSE
	if (LL_RCC_GetRTCClockSource() != LL_RCC_RTC_CLKSOURCE_LSE) {
#else
	if (LL_RCC_GetRTCClockSource() != LL_RCC_RTC_CLKSOURCE_LSI) {
#endif
		/* Save the RCC->BDCR and reset BackupDomain */
		uint32_t reg = RCC->BDCR & ~(RCC_BDCR_RTCSEL);
		LL_RCC_ForceBackupDomainReset();
		LL_RCC_ReleaseBackupDomainReset();
		RCC->BDCR = reg;
	}
#ifdef CONFIG_STM32_RTC_CLOCK_LSE
	while (LL_RCC_LSE_IsReady() == false) {
#else
	while (LL_RCC_LSI_IsReady() == false) {
#endif
		/* wait for LSI or LSE to be ready */
	}

#ifdef CONFIG_STM32_RTC_CLOCK_LSE
	LL_RCC_SetRTCClockSource(LL_RCC_RTC_CLKSOURCE_LSE);
#else
	LL_RCC_SetRTCClockSource(LL_RCC_RTC_CLKSOURCE_LSI);
#endif

	LL_RCC_EnableRTC();

	/* turn on RTC bus clock */
	const struct device *rcc_dev = DEVICE_DT_GET(STM32_CLOCK_CONTROL_NODE);
	if (clock_control_on(rcc_dev, (clock_control_subsys_t *)&rtc_clock_pclken) !=
	    0) {
		return -EIO;
	}
	/* Disable writing protection */
	LL_RTC_DisableWriteProtection(RTC);
	if (LL_RTC_EnterInitMode(RTC) == false) {
		return -EIO;
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

	/* Enable shadow register bypass， thus can
	   read the shadow register all the time. */
	LL_RTC_EnableShadowRegBypass(RTC);

	/* Restore writing protection */
	LL_RTC_EnableWriteProtection(RTC);

	/* Clear the alarm a flag */
	LL_RTC_ClearFlag_ALRA(RTC);

	IRQ_CONNECT(DT_IRQN(DT_NODELABEL(rtc)), DT_IRQ(DT_NODELABEL(rtc), priority),
		    rtc_timer_alarm_isr, NULL, 0);
	irq_enable(DT_IRQN(DT_NODELABEL(rtc)));

	return 0;
}

void sys_clock_set_timeout(int32_t ticks, bool idle)
{
	ARG_UNUSED(idle);

	if (IS_ENABLED(CONFIG_TICKLESS_KERNEL) == false) {
		return;
	}

	k_spinlock_key_t key = k_spin_lock(&rtc_timer_lock);

	uint32_t current = LL_RTC_TIME_GetSubSecond(RTC);
	/*
	 * The timeout can be a "negative" value, but it's an unsigned int type,
	 * So the timeout is UINT32_MAX - (ticks - current), Example:
	 * current is 0x1000, tickes is 0x2000,  0x1000 - 0x2000 = 0xFFFFF000
	 * now timeout set to 0xFFFFF000, when subsecond down to zero, subsecond
	 * will become to 0xFFFFFFFF. and will down to 0xFFFFF000 soon.
	 */
	uint32_t timeout = current - (uint32_t)ticks;

	if (LL_RTC_IsActiveFlag_ALRA(RTC)) {
		if (rtc_timer_diff(rtc_timer_backup, current) < RTC_TIMER_MINIMUM_VALUE) {
			/* interrupt happens or happens soon. It's impossible to set an alarm. */
			k_spin_unlock(&rtc_timer_lock, key);
			return;
		}
	}
	/* Set ticks minimum value if ticks is less than minimum */
	if ((uint32_t)ticks < RTC_TIMER_MINIMUM_VALUE) {
		timeout = current - RTC_TIMER_MINIMUM_VALUE;
	}

	rtc_timer_set_alarm(timeout);

	k_spin_unlock(&rtc_timer_lock, key);
}

uint32_t sys_clock_elapsed(void)
{
	if (IS_ENABLED(CONFIG_TICKLESS_KERNEL) == false) {
		return 0;
	}

	k_spinlock_key_t key = k_spin_lock(&rtc_timer_lock);

	/* return the difference subseconds since last rtc_timer_alarm_isr is called
	 */
	uint32_t current = LL_RTC_TIME_GetSubSecond(RTC);
	uint32_t elapsed = rtc_timer_diff(rtc_timer_backup, current);

	k_spin_unlock(&rtc_timer_lock, key);

	return elapsed;
}

uint32_t sys_clock_cycle_get_32(void)
{
	k_spinlock_key_t key = k_spin_lock(&rtc_timer_lock);

	uint64_t ticks = UINT32_MAX - LL_RTC_TIME_GetSubSecond(RTC);

	k_spin_unlock(&rtc_timer_lock, key);

	return ticks;
}
