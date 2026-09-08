# Raspberry Pi Pico 2 (RP2350, Cortex-M33).
#
# Unlike the STM32 boards (driven by the top-level clean-CMake spine + vendored
# CMSIS device packs), the RP2350 needs the pico-sdk for its boot/XIP/USB-CDC,
# so it is a SEPARATE pico-sdk project. The pico-sdk owns the toolchain, linker,
# startup and clock init. The tigris runtime + CMSIS-NN are compiled INTO the
# firmware here (not as standalone libs) so they inherit the pico-sdk M33 flags
# and CMSIS-Core, which is the pico-sdk convention.
#
# Build (needs PICO_SDK_PATH + a built picotool):
#   PICO_SDK_PATH=~/pico/pico-sdk cmake -S boards/pico2 -B build \
#     -Dpicotool_DIR=~/pico/picotool/install/lib/cmake/picotool
#   cmake --build build          # -> tigris_firmware.uf2
cmake_minimum_required(VERSION 3.13)

set(PICO_BOARD pico2)   # RP2350

# Match the STM32 build at -O2 (pico-sdk defaults its Release to -O3). Set before
# pico_sdk_init so the whole build, including the runtime + CMSIS-NN, is -O2.
set(CMAKE_C_FLAGS_RELEASE   "-O2 -DNDEBUG" CACHE STRING "" FORCE)
set(CMAKE_CXX_FLAGS_RELEASE "-O2 -DNDEBUG" CACHE STRING "" FORCE)

include($ENV{PICO_SDK_PATH}/pico_sdk_init.cmake)
project(tigris_pico2 C CXX ASM)
pico_sdk_init()

set(_root ${CMAKE_CURRENT_LIST_DIR}/../..)

# Pinned deps, populated (not built) so we can glob their sources into the
# firmware. Override offline with -DFETCHCONTENT_SOURCE_DIR_CMSIS_NN / _TIGRIS_RUNTIME.
include(FetchContent)
FetchContent_Declare(cmsis_nn
    GIT_REPOSITORY https://github.com/ARM-software/CMSIS-NN.git
    GIT_TAG 6d9d61d8a586c39160d0c1ba58f6948e4cf61ad0
    SOURCE_SUBDIR __tigris_no_add_subdir__)
FetchContent_Declare(tigris_runtime
    GIT_REPOSITORY https://github.com/raws-labs/tigris-runtime.git
    GIT_TAG 08f5d2b0a69ae73d4e0ec2eb0ac22476300a9971
    SOURCE_SUBDIR __tigris_no_add_subdir__)
FetchContent_MakeAvailable(cmsis_nn tigris_runtime)

file(GLOB          RT_SRC ${tigris_runtime_SOURCE_DIR}/src/*.c)
file(GLOB_RECURSE  NN_SRC ${cmsis_nn_SOURCE_DIR}/Source/*.c)

add_executable(tigris_firmware
    ${_root}/src/app/tigris_app.c
    ${_root}/src/hal/platform/pico2/tigris_hal_pico2.c
    ${_root}/examples/ds_cnn/tigris_codegen_core.c
    ${_root}/examples/ds_cnn/model_blob.c
    ${RT_SRC} ${NN_SRC})

target_include_directories(tigris_firmware PRIVATE
    ${_root}/include
    ${_root}/examples/ds_cnn
    ${tigris_runtime_SOURCE_DIR}/include
    ${cmsis_nn_SOURCE_DIR}/Include)

target_compile_definitions(tigris_firmware PRIVATE
    TIGRIS_HAS_CMSIS_NN
    # RP2350 has 520 KB SRAM; the 64 KB-budget ds_cnn example needs ~64 KB fast.
    TIGRIS_APP_FAST_ARENA_BYTES=73728
    TIGRIS_APP_SLOW_ARENA_BYTES=16384)

# Vendor code (CMSIS-NN): silence its warnings, do not fail the build on them.
set_source_files_properties(${NN_SRC} PROPERTIES COMPILE_OPTIONS "-w")

target_link_libraries(tigris_firmware pico_stdlib)
pico_enable_stdio_usb(tigris_firmware 1)
pico_enable_stdio_uart(tigris_firmware 0)
pico_add_extra_outputs(tigris_firmware)   # -> tigris_firmware.uf2
