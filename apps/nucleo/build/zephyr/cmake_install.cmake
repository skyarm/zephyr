# Install script for directory: /usr/local/src/zephyros/zephyr

# Set the install prefix
if(NOT DEFINED CMAKE_INSTALL_PREFIX)
  set(CMAKE_INSTALL_PREFIX "/usr/local")
endif()
string(REGEX REPLACE "/$" "" CMAKE_INSTALL_PREFIX "${CMAKE_INSTALL_PREFIX}")

# Set the install configuration name.
if(NOT DEFINED CMAKE_INSTALL_CONFIG_NAME)
  if(BUILD_TYPE)
    string(REGEX REPLACE "^[^A-Za-z0-9_]+" ""
           CMAKE_INSTALL_CONFIG_NAME "${BUILD_TYPE}")
  else()
    set(CMAKE_INSTALL_CONFIG_NAME "")
  endif()
  message(STATUS "Install configuration: \"${CMAKE_INSTALL_CONFIG_NAME}\"")
endif()

# Set the component getting installed.
if(NOT CMAKE_INSTALL_COMPONENT)
  if(COMPONENT)
    message(STATUS "Install component: \"${COMPONENT}\"")
    set(CMAKE_INSTALL_COMPONENT "${COMPONENT}")
  else()
    set(CMAKE_INSTALL_COMPONENT)
  endif()
endif()

# Is this installation the result of a crosscompile?
if(NOT DEFINED CMAKE_CROSSCOMPILING)
  set(CMAKE_CROSSCOMPILING "TRUE")
endif()

# Set default install directory permissions.
if(NOT DEFINED CMAKE_OBJDUMP)
  set(CMAKE_OBJDUMP "/usr/local/bin/arm-none-eabi-objdump")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for each subdirectory.
  include("/Users/zhangtao/Projects/zephyr/firmwares/nucleo/build/zephyr/arch/cmake_install.cmake")
  include("/Users/zhangtao/Projects/zephyr/firmwares/nucleo/build/zephyr/lib/cmake_install.cmake")
  include("/Users/zhangtao/Projects/zephyr/firmwares/nucleo/build/zephyr/soc/arm/cmake_install.cmake")
  include("/Users/zhangtao/Projects/zephyr/firmwares/nucleo/build/zephyr/boards/cmake_install.cmake")
  include("/Users/zhangtao/Projects/zephyr/firmwares/nucleo/build/zephyr/subsys/cmake_install.cmake")
  include("/Users/zhangtao/Projects/zephyr/firmwares/nucleo/build/zephyr/drivers/cmake_install.cmake")
  include("/Users/zhangtao/Projects/zephyr/firmwares/nucleo/build/modules/cmsis/cmake_install.cmake")
  include("/Users/zhangtao/Projects/zephyr/firmwares/nucleo/build/modules/atmel/cmake_install.cmake")
  include("/Users/zhangtao/Projects/zephyr/firmwares/nucleo/build/modules/altera/cmake_install.cmake")
  include("/Users/zhangtao/Projects/zephyr/firmwares/nucleo/build/modules/canopennode/cmake_install.cmake")
  include("/Users/zhangtao/Projects/zephyr/firmwares/nucleo/build/modules/civetweb/cmake_install.cmake")
  include("/Users/zhangtao/Projects/zephyr/firmwares/nucleo/build/modules/espressif/cmake_install.cmake")
  include("/Users/zhangtao/Projects/zephyr/firmwares/nucleo/build/modules/fatfs/cmake_install.cmake")
  include("/Users/zhangtao/Projects/zephyr/firmwares/nucleo/build/modules/cypress/cmake_install.cmake")
  include("/Users/zhangtao/Projects/zephyr/firmwares/nucleo/build/modules/infineon/cmake_install.cmake")
  include("/Users/zhangtao/Projects/zephyr/firmwares/nucleo/build/modules/hal_nordic/cmake_install.cmake")
  include("/Users/zhangtao/Projects/zephyr/firmwares/nucleo/build/modules/openisa/cmake_install.cmake")
  include("/Users/zhangtao/Projects/zephyr/firmwares/nucleo/build/modules/nuvoton/cmake_install.cmake")
  include("/Users/zhangtao/Projects/zephyr/firmwares/nucleo/build/modules/microchip/cmake_install.cmake")
  include("/Users/zhangtao/Projects/zephyr/firmwares/nucleo/build/modules/silabs/cmake_install.cmake")
  include("/Users/zhangtao/Projects/zephyr/firmwares/nucleo/build/modules/st/cmake_install.cmake")
  include("/Users/zhangtao/Projects/zephyr/firmwares/nucleo/build/modules/stm32/cmake_install.cmake")
  include("/Users/zhangtao/Projects/zephyr/firmwares/nucleo/build/modules/ti/cmake_install.cmake")
  include("/Users/zhangtao/Projects/zephyr/firmwares/nucleo/build/modules/libmetal/cmake_install.cmake")
  include("/Users/zhangtao/Projects/zephyr/firmwares/nucleo/build/modules/quicklogic/cmake_install.cmake")
  include("/Users/zhangtao/Projects/zephyr/firmwares/nucleo/build/modules/lvgl/cmake_install.cmake")
  include("/Users/zhangtao/Projects/zephyr/firmwares/nucleo/build/modules/mbedtls/cmake_install.cmake")
  include("/Users/zhangtao/Projects/zephyr/firmwares/nucleo/build/modules/mcuboot/cmake_install.cmake")
  include("/Users/zhangtao/Projects/zephyr/firmwares/nucleo/build/modules/mcumgr/cmake_install.cmake")
  include("/Users/zhangtao/Projects/zephyr/firmwares/nucleo/build/modules/nxp/cmake_install.cmake")
  include("/Users/zhangtao/Projects/zephyr/firmwares/nucleo/build/modules/open-amp/cmake_install.cmake")
  include("/Users/zhangtao/Projects/zephyr/firmwares/nucleo/build/modules/loramac-node/cmake_install.cmake")
  include("/Users/zhangtao/Projects/zephyr/firmwares/nucleo/build/modules/openthread/cmake_install.cmake")
  include("/Users/zhangtao/Projects/zephyr/firmwares/nucleo/build/modules/segger/cmake_install.cmake")
  include("/Users/zhangtao/Projects/zephyr/firmwares/nucleo/build/modules/sof/cmake_install.cmake")
  include("/Users/zhangtao/Projects/zephyr/firmwares/nucleo/build/modules/tinycbor/cmake_install.cmake")
  include("/Users/zhangtao/Projects/zephyr/firmwares/nucleo/build/modules/tinycrypt/cmake_install.cmake")
  include("/Users/zhangtao/Projects/zephyr/firmwares/nucleo/build/modules/littlefs/cmake_install.cmake")
  include("/Users/zhangtao/Projects/zephyr/firmwares/nucleo/build/modules/mipi-sys-t/cmake_install.cmake")
  include("/Users/zhangtao/Projects/zephyr/firmwares/nucleo/build/modules/nrf_hw_models/cmake_install.cmake")
  include("/Users/zhangtao/Projects/zephyr/firmwares/nucleo/build/modules/xtensa/cmake_install.cmake")
  include("/Users/zhangtao/Projects/zephyr/firmwares/nucleo/build/modules/trusted-firmware-m/cmake_install.cmake")
  include("/Users/zhangtao/Projects/zephyr/firmwares/nucleo/build/modules/nanopb/cmake_install.cmake")
  include("/Users/zhangtao/Projects/zephyr/firmwares/nucleo/build/modules/tensorflow/cmake_install.cmake")
  include("/Users/zhangtao/Projects/zephyr/firmwares/nucleo/build/modules/ipm/cmake_install.cmake")
  include("/Users/zhangtao/Projects/zephyr/firmwares/nucleo/build/zephyr/kernel/cmake_install.cmake")
  include("/Users/zhangtao/Projects/zephyr/firmwares/nucleo/build/zephyr/cmake/flash/cmake_install.cmake")
  include("/Users/zhangtao/Projects/zephyr/firmwares/nucleo/build/zephyr/cmake/usage/cmake_install.cmake")
  include("/Users/zhangtao/Projects/zephyr/firmwares/nucleo/build/zephyr/cmake/reports/cmake_install.cmake")

endif()

