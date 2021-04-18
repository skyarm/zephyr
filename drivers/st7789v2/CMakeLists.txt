# SPDX-License-Identifier: Apache-2.0

if(CONFIG_ST7789V2)
  # Add hello_world_driver.h to the set of global include paths.
  zephyr_include_directories(.)

  zephyr_library()
  zephyr_library_sources(
    st7789v2.c
    )
endif()
