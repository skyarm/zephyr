/*
 * Copyright (c) 2020 Teslabs Engineering S.L.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#if DT_NODE_HAS_STATUS(DT_NODELABEL(sram1), okay)
GROUP_START(SRAM1)

	SECTION_PROLOGUE(_STM32_SDRAM1_SECTION_NAME, (NOLOAD),)
	{
		*(.stm32_sram1)
		*(".stm32_sram1.*")
	} GROUP_LINK_IN(SRAM1)

GROUP_END(SRAM1)
#endif

#if DT_NODE_HAS_STATUS(DT_NODELABEL(sram2), okay)
GROUP_START(SRAM2)

	SECTION_PROLOGUE(_STM32_SRAM2_SECTION_NAME, (NOLOAD),)
	{
		*(.stm32_sram2)
		*(".stm32_sram2.*")
	} GROUP_LINK_IN(SRAM2)

GROUP_END(SRAM2)
#endif
