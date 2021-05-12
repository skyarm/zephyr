# SPDX-License-Identifier: Apache-2.0

cmake_minimum_required(VERSION 3.13.1)

list(APPEND ZEPHYR_EXTRA_MODULES
  /Users/zhangtao/Projects/zephyr/drivers/stm32_fmc_sram
)
set(BOARD_ROOT /Users/zhangtao/Projects/zephyr)
set(BOARD disco_l496ag)

include_directories(/usr/local/src/zephyr/modules/fs/littlefs)
include_directories(/usr/local/src/zephyr/modules/fs/fatfs/include)

find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})
project(disco)
target_sources(app PRIVATE src/main.c)
