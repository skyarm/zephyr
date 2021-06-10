# SPDX-License-Identifier: Apache-2.0

cmake_minimum_required(VERSION 3.13.1)

list(APPEND BOARD_ROOT /Users/zhangtao/Projects/zephyr)
set(BOARD nucleo_wl55jc2_cm4)

list(APPEND ZEPHYR_EXTRA_MODULES
  /Users/zhangtao/Projects/zephyr/drivers/ipm
)

list(APPEND ZEPHYR_EXTRA_MODULES
  /Users/zhangtao/Projects/zephyr/drivers/timer
)

find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})
project(nucleo)

add_subdirectory(lorawan/sysdep)
add_subdirectory(lorawan/crypto)
add_subdirectory(lorawan/radio)
add_subdirectory(lorawan/mac)
add_subdirectory(lorawan/lmh)

target_sources(app PRIVATE sources/main.c)
target_sources(app PRIVATE sources/network.c)