# SPDX-License-Identifier: Apache-2.0

if(CONFIG_BOARD_OUR_BOARD_NRF5340_CPUAPP)
  board_runner_args(nrfutil "--ext-mem-config-file=${BOARD_DIR}/support/my_board_qspi_nrfutil_config.json")
  board_runner_args(jlink "--device=nrf5340_xxaa_app" "--speed=4000")
endif()

include(${ZEPHYR_BASE}/boards/common/nrfutil.board.cmake)
include(${ZEPHYR_BASE}/boards/common/nrfjprog.board.cmake)
include(${ZEPHYR_BASE}/boards/common/jlink.board.cmake)
