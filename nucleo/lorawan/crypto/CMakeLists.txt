# SPDX-License-Identifier: Apache-2.0

zephyr_include_directories(.)

zephyr_sources(
  lorawan_aes.c
  cmac.c
  soft-se.c
)
