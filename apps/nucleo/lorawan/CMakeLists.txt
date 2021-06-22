# SPDX-License-Identifier: Apache-2.0

add_subdirectory(sysdep)
add_subdirectory(crypto)
add_subdirectory(radio)
add_subdirectory(mac)

zephyr_include_directories(.)

zephyr_sources(
  lorawan_node.c
)
