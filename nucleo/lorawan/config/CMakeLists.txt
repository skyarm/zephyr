# SPDX-License-Identifier: Apache-2.0

zephyr_include_directories(.)

zephyr_sources(
  systime.c
  timer.c
  rtctime.c
  utilities.c
  subghz.c
  lora_info.c
)
