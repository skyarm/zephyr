# SPDX-License-Identifier: Apache-2.0

zephyr_include_directories(.)
zephyr_include_directories(Region)

zephyr_sources(
  LoRaMac.c
  LoRaMacAdr.c
  LoRaMacClassB.c
  LoRaMacCommands.c
  LoRaMacConfirmQueue.c
  LoRaMacCrypto.c
  LoRaMacParser.c
  LoRaMacSerializer.c
  Region/Region.c
  Region/RegionCommon.c
  Region/RegionCN470.c
)
