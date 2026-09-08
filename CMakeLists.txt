cmake_minimum_required(VERSION 3.20)

# A cross toolchain must be selected at configure time. STM32 boards and the
# no-board "spine" build use the bundled generic arm-none-eabi toolchain; pico2
# (RP2350) drives its own build through pico-sdk and supplies that toolchain in
# its board file, so only skip the default for it.
if(NOT CMAKE_TOOLCHAIN_FILE AND NOT TIGRIS_BOARD STREQUAL "pico2")
    set(CMAKE_TOOLCHAIN_FILE
        "${CMAKE_CURRENT_SOURCE_DIR}/cmake/toolchains/arm-none-eabi.cmake")
endif()

project(tigris_cortex_m VERSION 0.1.0 LANGUAGES C ASM)

set(TIGRIS_CM_ROOT ${CMAKE_CURRENT_SOURCE_DIR})

# Pico 2 / RP2350 is a separate pico-sdk project (the SDK owns the toolchain,
# linker, startup and clock), not a board file included here.
if(TIGRIS_BOARD STREQUAL "pico2")
    message(FATAL_ERROR
        "Pico 2 / RP2350 builds through its own pico-sdk project:\n"
        "  PICO_SDK_PATH=/path/to/pico-sdk cmake -S boards/pico2 -B build\n"
        "See boards/pico2/CMakeLists.txt.")
endif()

include(cmake/cpu.cmake)

# Resolve the core (and thus the CPU flags) before the spine, so the runtime and
# cmsis-nn compile for the selected target. A board fixes the core; without one,
# TIGRIS_TARGET_ARCH drives it and defaults to cortex-m4 for a standalone spine.
if(TIGRIS_BOARD STREQUAL "nucleo_f446re")
    set(TIGRIS_TARGET_ARCH cortex-m4)
elseif(TIGRIS_BOARD STREQUAL "nucleo_h753zi")
    set(TIGRIS_TARGET_ARCH cortex-m7)
elseif(NOT TIGRIS_TARGET_ARCH)
    set(TIGRIS_TARGET_ARCH cortex-m4)
endif()

# Global compile flags for STM32 / spine builds (pico2 gets its own from
# pico-sdk). -O2 is the measured sweet spot for the CMSIS-NN kernels; the section
# flags let --gc-sections drop unused code at link.
if(NOT TIGRIS_BOARD STREQUAL "pico2")
    tigris_cpu_flags(${TIGRIS_TARGET_ARCH} TIGRIS_CPU_FLAGS)
    add_compile_options(${TIGRIS_CPU_FLAGS} -O2 -ffunction-sections -fdata-sections)
endif()

# Dependency spine: fetch CMSIS sources, define the `cmsis-nn` target the runtime
# links, then build the runtime with the CMSIS-NN backend enabled.
include(cmake/deps.cmake)
include(cmake/cmsis_nn.cmake)
set(TIGRIS_HAS_CMSIS_NN ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(tigris_runtime)

# Board bring-up + minimal firmware. The STM32 boards share one firmware-target
# helper; each board file just supplies its device pack, linker and platform.
include(cmake/stm32_firmware.cmake)
if(TIGRIS_BOARD)
    if(NOT EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/cmake/boards/${TIGRIS_BOARD}.cmake")
        message(FATAL_ERROR
            "Unknown TIGRIS_BOARD '${TIGRIS_BOARD}' "
            "(expected nucleo_f446re | nucleo_h753zi | pico2)")
    endif()
    include(cmake/boards/${TIGRIS_BOARD}.cmake)
endif()
