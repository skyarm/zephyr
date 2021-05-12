/*
 * Copyright (c) 2020 Teslabs Engineering S.L.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#define DT_DRV_COMPAT st_stm32_fmc_sram

#include <device.h>
#include <drivers/clock_control/stm32_clock_control.h>
#include <drivers/clock_control.h>
#include <pinmux/stm32/pinmux_stm32.h>
#include <logging/log.h>
#include <soc.h>

LOG_MODULE_REGISTER(stm32_fmc_sram, LOG_LEVEL_INF);

/** FMC SDRAM controller configuration fields. */
struct stm32_fmc_sram_config {
  uint32_t fmc;
  struct stm32_pclken pclken;
  const struct soc_gpio_pinctrl *pinctrl;
  size_t pinctrl_len;
  FMC_Bank1_TypeDef init;
  FMC_NORSRAM_TimingTypeDef timing;
  FMC_NORSRAM_InitTypeDef ext_init;
  FMC_NORSRAM_TimingTypeDef *ext_timing;
};

static int stm32_fmc_sram_init(const struct device *dev) {
  const struct stm32_fmc_sram_config *config = dev->config;

  int rc;

  if (clock_control_on(DEVICE_DT_GET(STM32_CLOCK_CONTROL_NODE),
                       (clock_control_subsys_t)&config->pclken) != 0) {
    LOG_ERR("Could not enable FMC clock");
    return -EIO;
  }

  /* Configure dt provided device signals when available */
  rc = stm32_dt_pinctrl_configure(config->pinctrl, config->pinctrl_len,
                                   config->fmc);
  if (rc < 0) {
    LOG_ERR("FMC pinctrl setup failed (%d)", rc);
    return rc;
  }

  /* Initialize SRAM timing Interface */
  (void)FMC_NORSRAM_Timing_Init(&config->init, &config->timing,
                                config->init.NSBank);
  /* Initialize SRAM extended mode timing Interface */
  (void)FMC_NORSRAM_Extended_Timing_Init(&config->init, &config->timing,
                                         config->init.NSBank,
                                         config->init.ExtendedMode);
  /* Enable the NORSRAM device */
  __FMC_NORSRAM_ENABLE(&config->init, config->init.NSBank);

  return 0;
}

/** SDRAM configuration. */
static const struct stm32_fmc_sram_config config = {
    .init =
        {
            .SDBank = DT_REG_ADDR(node_id),
            .ColumnBitsNumber = DT_PROP_BY_IDX(node_id, st_sram_control, 0),
            .RowBitsNumber = DT_PROP_BY_IDX(node_id, st_sram_control, 1),
            .MemoryDataWidth = DT_PROP_BY_IDX(node_id, st_sram_control, 2),
            .InternalBankNumber = DT_PROP_BY_IDX(node_id, st_sram_control, 3),
            .CASLatency = DT_PROP_BY_IDX(node_id, st_sram_control, 4),
            .WriteProtection = FMC_SDRAM_WRITE_PROTECTION_DISABLE,
            .SDClockPeriod = DT_PROP_BY_IDX(node_id, st_sram_control, 5),
            .ReadBurst = DT_PROP_BY_IDX(node_id, st_sram_control, 6),
            .ReadPipeDelay = DT_PROP_BY_IDX(node_id, st_sram_control, 7),
        },
    .timing =
        {
            .LoadToActiveDelay = DT_PROP_BY_IDX(node_id, st_sram_timing, 0),
            .ExitSelfRefreshDelay = DT_PROP_BY_IDX(node_id, st_sram_timing, 1),
            .SelfRefreshTime = DT_PROP_BY_IDX(node_id, st_sram_timing, 2),
            .RowCycleDelay = DT_PROP_BY_IDX(node_id, st_sram_timing, 3),
            .WriteRecoveryTime = DT_PROP_BY_IDX(node_id, st_sram_timing, 4),
            .RPDelay = DT_PROP_BY_IDX(node_id, st_sram_timing, 5),
            .RCDDelay = DT_PROP_BY_IDX(node_id, st_sram_timing, 6),
        },
    .ext_timing = {
        .LoadToActiveDelay = DT_PROP_BY_IDX(node_id, st_sram_timing, 0),
        .ExitSelfRefreshDelay = DT_PROP_BY_IDX(node_id, st_sram_timing, 1),
        .SelfRefreshTime = DT_PROP_BY_IDX(node_id, st_sram_timing, 2),
        .RowCycleDelay = DT_PROP_BY_IDX(node_id, st_sram_timing, 3),
        .WriteRecoveryTime = DT_PROP_BY_IDX(node_id, st_sram_timing, 4),
        .RPDelay = DT_PROP_BY_IDX(node_id, st_sram_timing, 5),
        .RCDDelay = DT_PROP_BY_IDX(node_id, st_sram_timing, 6),
    }};

DEVICE_DT_INST_DEFINE(0, stm32_fmc_sram_init, device_pm_control_nop, NULL,
                      &config, POST_KERNEL, CONFIG_MEMC_INIT_PRIORITY, NULL);
