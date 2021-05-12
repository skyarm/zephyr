# SPDX-License-Identifier: Apache-2.0

zephyr_include_directories(.)
zephyr_include_directories(Packages)

zephyr_sources(
  LmHandler.c
  Packages/LmhpCompliance.c
  Packages/LmhpClockSync.h
  NvmCtxMgmt.c
  Packages/LmhpRemoteMcastSetup.c
  Packages/LmhpFragmentation.c
  Packages/LmhpClockSync.c
)
