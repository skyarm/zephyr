/*
 * Copyright (c) 2019 Jan Van Winkel <jan.van_winkel@dxplore.eu>
 *
 * Based on ST7789V sample:
 * Copyright (c) 2019 Marc Reilly
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#include <logging/log.h>
LOG_MODULE_REGISTER(deamon, LOG_LEVEL_INF);

#include <device.h>
#include <drivers/gpio.h>
#include <math.h>
#include <stdio.h>
#include <sys/byteorder.h>
#include <zephyr.h>

#include <stm32wlxx_hal.h>

void main(void) {
    HAL_PWREx_ReleaseCore(PWR_CORE_CPU2);
    for (;;) {
        k_msleep(10000);
        LOG_INF("Waiting for CPU2 to boot from[%X]", LL_FLASH_GetC2BootResetVect());
    }
}
