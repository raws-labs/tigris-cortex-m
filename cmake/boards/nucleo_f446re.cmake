# NUCLEO-F446RE (STM32F446RE, Cortex-M4F). Single 128 KB SRAM.
tigris_add_stm32_firmware(
    DEVICE_NAME cmsis_device_f4
    DEVICE_REPO https://github.com/STMicroelectronics/cmsis-device-f4.git
    DEVICE_TAG  3c77349ce04c8af401454cc51f85ea9a50e34fc1
    DEFINE      STM32F446xx
    STARTUP     Source/Templates/gcc/startup_stm32f446xx.s
    SYSTEM      Source/Templates/system_stm32f4xx.c
    PLATFORM    nucleo_f446re/tigris_hal_f446.c
    LINKER      nucleo_f446re/stm32f446re.ld
    FAST 73728 SLOW 16384)
