# Raspberry Pi Pico 2 (RP2350, Cortex-M33).
#
# Unlike the STM32 boards (driven by the top-level clean-CMake spine + vendored
# CMSIS device packs), the RP2350 needs the pico-sdk for its boot/XIP/USB-CDC,
# so it is a SEPARATE pico-sdk project. The pico-sdk owns the toolchain, linker,
# startup and clock init. The tigris runtime is compiled INTO the firmware here
# (not as a standalone lib) so it inherits the pico-sdk M33 flags, which is the
# pico-sdk convention; CMSIS-NN is built by its own CMake project inside this
# one, with the same toolchain flags and pico-sdk's CMSIS-Core.
#
# Build (needs PICO_SDK_PATH + a built picotool):
#   PICO_SDK_PATH=~/pico/pico-sdk cmake -S boards/pico2 -B build \
#     -Dpicotool_DIR=~/pico/picotool/install/lib/cmake/picotool
#   cmake --build build          # -> tigris_firmware.uf2
cmake_minimum_required(VERSION 3.25)

set(PICO_BOARD pico2)   # RP2350

# Match the STM32 build at -O2 (pico-sdk defaults its Release to -O3). Set before
# pico_sdk_init so the whole build, including the runtime + CMSIS-NN, is -O2.
set(CMAKE_C_FLAGS_RELEASE   "-O2 -DNDEBUG" CACHE STRING "" FORCE)
set(CMAKE_CXX_FLAGS_RELEASE "-O2 -DNDEBUG" CACHE STRING "" FORCE)

include($ENV{PICO_SDK_PATH}/pico_sdk_init.cmake)
project(tigris_pico2 C CXX ASM)
pico_sdk_init()

set(_root ${CMAKE_CURRENT_LIST_DIR}/../..)

# Pinned deps, populated (not built by FetchContent): the runtime sources are
# globbed into the firmware, CMSIS-NN is added below. Override offline with
# -DFETCHCONTENT_SOURCE_DIR_CMSIS_NN / _TIGRIS_RUNTIME.
include(FetchContent)
FetchContent_Declare(cmsis_nn
    GIT_REPOSITORY https://github.com/ARM-software/CMSIS-NN.git
    GIT_TAG 13c97dbb6f781d4aab38ed34e6e441f42b79aff4
    SOURCE_SUBDIR __tigris_no_add_subdir__)
FetchContent_Declare(tigris_runtime
    GIT_REPOSITORY https://github.com/raws-labs/tigris-runtime.git
    GIT_TAG 65c4d6290a1959c7f956e0b492a8997dd20fda06
    SOURCE_SUBDIR __tigris_no_add_subdir__)
FetchContent_MakeAvailable(cmsis_nn tigris_runtime)

file(GLOB RT_SRC ${tigris_runtime_SOURCE_DIR}/src/*.c)

# CMSIS-NN's own project selects its sources; -O2 as above (its default is
# -Ofast), and SYSTEM plus -w keep its warnings out of the build.
set(CMSIS_OPTIMIZATION_LEVEL "-O2" CACHE STRING "" FORCE)
set(CMSIS_PATH $ENV{PICO_SDK_PATH}/src/rp2_common/cmsis/stub)
add_subdirectory(${cmsis_nn_SOURCE_DIR} ${cmsis_nn_BINARY_DIR} SYSTEM)
target_compile_options(cmsis-nn PRIVATE -w)

add_executable(tigris_firmware
    ${_root}/src/app/tigris_app.c
    ${_root}/src/hal/platform/pico2/tigris_hal_pico2.c
    ${_root}/examples/ds_cnn/tigris_codegen_core.c
    ${_root}/examples/ds_cnn/model_blob.c
    ${RT_SRC})

target_include_directories(tigris_firmware PRIVATE
    ${_root}/include
    ${_root}/examples/ds_cnn
    ${tigris_runtime_SOURCE_DIR}/include)

target_compile_definitions(tigris_firmware PRIVATE
    TIGRIS_HAS_CMSIS_NN
    # RP2350 has 520 KB SRAM; the arenas match the STM32 board files.
    TIGRIS_APP_FAST_ARENA_BYTES=73728
    TIGRIS_APP_SLOW_ARENA_BYTES=16384)

# The runtime's own build compiles it with this flag: float results must not
# depend on whether the compiler fuses multiply-adds.
set_source_files_properties(${RT_SRC} PROPERTIES COMPILE_OPTIONS "-ffp-contract=off")

target_link_libraries(tigris_firmware pico_stdlib cmsis-nn)
pico_enable_stdio_usb(tigris_firmware 1)
pico_enable_stdio_uart(tigris_firmware 0)
pico_add_extra_outputs(tigris_firmware)   # -> tigris_firmware.uf2
