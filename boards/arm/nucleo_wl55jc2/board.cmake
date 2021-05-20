# SPDX-License-Identifier: Apache-2.0

if(CONFIG_BOARD_NUCLEO_WL55JC2_CM0)
    board_runner_args(openocd "--config=${BOARD_DIR}/support/openocd_nucleo_wl55jc2_m0.cfg")
elseif(CONFIG_BOARD_NUCLEO_WL55JC2_CM4)
    board_runner_args(openocd "--config=${BOARD_DIR}/support/openocd_nucleo_wl55jc2_m0.cfg")
endif()

include(${ZEPHYR_BASE}/boards/common/openocd.board.cmake)
