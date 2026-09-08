# Host compile guard (no hardware).
#
# Compiles the portable layer (the app, the HAL header, the example) for the
# native host, so a board register or device header that leaked into the
# portable code fails here instead of only on a cross-build. The board runtime
# paths themselves are verified on real silicon; this is the fast CI check.
#
#   cmake -S boards/host -B build-host && cmake --build build-host
cmake_minimum_required(VERSION 3.20)
project(tigris_cm_host_check C)

set(_root ${CMAKE_CURRENT_LIST_DIR}/../..)

# The runtime is fetched only for its headers (the app includes them). Override
# offline with -DFETCHCONTENT_SOURCE_DIR_TIGRIS_RUNTIME=<path>.
include(FetchContent)
FetchContent_Declare(tigris_runtime
    GIT_REPOSITORY https://github.com/raws-labs/tigris-runtime.git
    GIT_TAG 08f5d2b0a69ae73d4e0ec2eb0ac22476300a9971
    SOURCE_SUBDIR __tigris_no_add_subdir__)
FetchContent_MakeAvailable(tigris_runtime)

# Object library only: we compile, we do not link a backend (CMSIS-NN is
# ARM-only). Compilation alone catches the leakage this guard is for.
add_library(tigris_cm_host_check OBJECT
    ${_root}/src/app/tigris_app.c
    ${_root}/src/hal/platform/host/tigris_hal_host.c
    ${_root}/examples/ds_cnn/tigris_codegen_core.c
    ${_root}/examples/ds_cnn/model_blob.c)

target_include_directories(tigris_cm_host_check PRIVATE
    ${_root}/include
    ${_root}/examples/ds_cnn
    ${tigris_runtime_SOURCE_DIR}/include)

# The app's arena-sizing calls the CMSIS-NN helper (declaration only here).
target_compile_definitions(tigris_cm_host_check PRIVATE TIGRIS_HAS_CMSIS_NN)
target_compile_options(tigris_cm_host_check PRIVATE -Wall -Wextra)
