# SPDX-License-Identifier: Apache-2.0

cmake_minimum_required(VERSION 3.13.1)

#list(APPEND ZEPHYR_EXTRA_MODULES
#  /Users/zhangtao/Projects/zephyr/drivers/stm32_fmc_sram
#)
list(APPEND BOARD_ROOT /Users/zhangtao/Projects/zephyr)
set(BOARD nucleo_wl55jc2m4)

find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})
project(nucleo_cm4)

add_subdirectory(lorawan/config)
add_subdirectory(lorawan/crypto)
add_subdirectory(lorawan/radio)
add_subdirectory(lorawan/mac)
add_subdirectory(lorawan/lmh)

target_sources(app PRIVATE src/main.c)

