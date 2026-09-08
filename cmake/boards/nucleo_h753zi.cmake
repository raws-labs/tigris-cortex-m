# NUCLEO-H753ZI (STM32H753ZI, Cortex-M7). Multi-bank map: arenas in AXISRAM
# (512 KB), stack in DTCM. That placement is fixed in our linker (stm32h753zi.ld).
tigris_add_stm32_firmware(
    DEVICE_NAME cmsis_device_h7
    DEVICE_REPO https://github.com/STMicroelectronics/cmsis-device-h7.git
    DEVICE_TAG  de8243d2c15f87936f28a49fcd9e6f5ba10fc233
    DEFINE      STM32H753xx
    STARTUP     Source/Templates/gcc/startup_stm32h753xx.s
    SYSTEM      Source/Templates/system_stm32h7xx.c
    PLATFORM    nucleo_h753zi/tigris_hal_h753.c
    LINKER      nucleo_h753zi/stm32h753zi.ld
    FAST 73728 SLOW 16384)
