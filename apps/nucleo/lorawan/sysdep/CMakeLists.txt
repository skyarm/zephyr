# SPDX-License-Identifier: Apache-2.0


zephyr_include_directories(.)
zephyr_include_directories(../config)

zephyr_sources(
  systime.c
  timer.c
  rtctime.c
  utilities.c
  subghz.c
)
