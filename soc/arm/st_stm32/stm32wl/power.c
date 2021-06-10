/*
 * Copyright (c) 2019 STMicroelectronics.
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#include <zephyr.h>
#include <pm/pm.h>
#include <soc.h>
#include <init.h>

#include <stm32wlxx_ll_utils.h>
#include <stm32wlxx_ll_bus.h>
#include <stm32wlxx_ll_cortex.h>
#include <stm32wlxx_ll_pwr.h>
#include <stm32wlxx_ll_rcc.h>
#include <clock_control/clock_stm32_ll_common.h>

#include <logging/log.h>
LOG_MODULE_DECLARE(soc, CONFIG_SOC_LOG_LEVEL);

static inline void suspend_systick(void) {
  /* Disable SysTick Interrupt */
  CLEAR_BIT(SysTick->CTRL, SysTick_CTRL_TICKINT_Msk);
}

static inline void resume_systick(void) {
  /* Enable SysTick Interrupt */
  SET_BIT(SysTick->CTRL, SysTick_CTRL_TICKINT_Msk);
}

/* Invoke Low Power/System Off specific Tasks */
void pm_power_state_set(struct pm_state_info info)
{
	if (info.state != PM_STATE_SUSPEND_TO_IDLE) {
		LOG_DBG("Unsupported power state %u", info.state);
		return;
	}

	switch (info.substate_id) {
	case 1: /* this corresponds to the STOP0 mode: */
#ifndef		CONFIG_CPU_CORTEX_M0PLUS
		LL_PWR_ClearFlag_C1STOP_C1STB();
#endif
		/* ensure MSI is the wake-up system clock */
		LL_RCC_SetClkAfterWakeFromStop(LL_RCC_STOP_WAKEUPCLOCK_MSI);
		/* enter STOP0 mode */
		LL_PWR_SetPowerMode(LL_PWR_MODE_STOP0);
		LL_LPM_EnableDeepSleep();
		/* enter SLEEP mode : WFE or WFI */
		printk("Enter STOP0 mode\n");
		k_cpu_idle();
		break;
	case 2: /* this corresponds to the STOP1 mode: */
#ifndef         CONFIG_CPU_CORTEX_M0PLUS
                LL_PWR_ClearFlag_C1STOP_C1STB();
#endif
		/* ensure MSI is the wake-up system clock */
		LL_RCC_SetClkAfterWakeFromStop(LL_RCC_STOP_WAKEUPCLOCK_MSI);
		/* enter STOP1 mode */
		LL_PWR_SetPowerMode(LL_PWR_MODE_STOP1);
		LL_LPM_EnableDeepSleep();
		/* enter SLEEP mode : WFE or WFI */
		printk("Enter STOP1 mode\n");
		k_cpu_idle();
		break;
	case 3: /* this corresponds to the STOP2 mode: */
#ifndef         CONFIG_CPU_CORTEX_M0PLUS
                LL_PWR_ClearFlag_C1STOP_C1STB();
#endif
		/* ensure MSI is the wake-up system clock */
		LL_RCC_SetClkAfterWakeFromStop(LL_RCC_STOP_WAKEUPCLOCK_MSI);
#ifdef PWR_CR1_RRSTP
		LL_PWR_DisableSRAM3Retention();
#endif /* PWR_CR1_RRSTP */
		/* enter STOP2 mode */
		LL_PWR_SetPowerMode(LL_PWR_MODE_STOP2);
		LL_LPM_EnableDeepSleep();
		/* enter SLEEP mode : WFE or WFI */
		printk("Enter STOP2 mode\n");
		k_cpu_idle();
		break;
	default:
		LOG_DBG("Unsupported power substate-id %u", info.substate_id);
		break;
	}
}

/* Handle SOC specific activity after Low Power Mode Exit */
void pm_power_state_exit_post_ops(struct pm_state_info info)
{
	if (info.state != PM_STATE_SUSPEND_TO_IDLE) {
		LOG_DBG("Unsupported power state %u", info.state);
	} else {
		switch (info.substate_id) {
		case 1:	/* STOP0 */
			__fallthrough;
		case 2:	/* STOP1 */
			__fallthrough;
		case 3:	/* STOP2 */
			LL_LPM_DisableSleepOnExit();
			LL_LPM_EnableSleep();
			break;
		default:
			LOG_DBG("Unsupported power substate-id %u",
				info.substate_id);
			break;
		}
		/* need to restore the clock */
		stm32_clock_control_init(NULL);
		LL_PWR_EnableBkUpAccess();
  		while (LL_PWR_IsEnabledBkUpAccess() == 0) {
  		}
		printk("Exit STOP mode\n");
	}

	/*
	 * System is now in active mode.
	 * Reenable interrupts which were disabled
	 * when OS started idling code.
	 */
	irq_unlock(0);
}

/* Initialize STM32 Power */
static int stm32_power_init(const struct device *dev)
{
	ARG_UNUSED(dev);

#ifdef CONFIG_DEBUG
	/* Enable the Debug Module during STOP mode */
	LL_DBGMCU_EnableDBGStopMode();
#endif /* CONFIG_DEBUG */

	return 0;
}

SYS_INIT(stm32_power_init, POST_KERNEL, CONFIG_KERNEL_INIT_PRIORITY_DEFAULT);
