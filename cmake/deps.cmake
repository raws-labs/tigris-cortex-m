# Pinned upstream dependencies, fetched by exact commit.
#
# For an offline / local-mirror build, override any source with
#   -DFETCHCONTENT_SOURCE_DIR_<NAME>=<path>
# where NAME is upper-cased: CMSIS_NN, CMSIS_CORE, TIGRIS_RUNTIME.
include(FetchContent)

# CMSIS-NN and CMSIS-Core are fetched as SOURCES ONLY. SOURCE_SUBDIR points at a
# nonexistent directory so FetchContent_MakeAvailable populates the trees without
# add_subdirectory'ing their own CMake projects; we build our own `cmsis-nn`
# target from these sources in cmake/cmsis_nn.cmake.
FetchContent_Declare(cmsis_nn
    GIT_REPOSITORY https://github.com/ARM-software/CMSIS-NN.git
    GIT_TAG 6d9d61d8a586c39160d0c1ba58f6948e4cf61ad0  # v7.0.0, matches TFLM's pin
    SOURCE_SUBDIR __tigris_no_add_subdir__)
FetchContent_Declare(cmsis_core
    GIT_REPOSITORY https://github.com/ARM-software/CMSIS_6.git
    GIT_TAG 45dab712ad84f8cbbf2b7bfc089c19088507df6f  # CMSIS-Core (core_cm*.h)
    SOURCE_SUBDIR __tigris_no_add_subdir__)
FetchContent_MakeAvailable(cmsis_nn cmsis_core)

# The portable runtime core. Declared here; the top-level CMakeLists calls
# FetchContent_MakeAvailable(tigris_runtime) AFTER cmake/cmsis_nn.cmake defines
# the `cmsis-nn` target that the runtime's TIGRIS_HAS_CMSIS_NN option links.
FetchContent_Declare(tigris_runtime
    GIT_REPOSITORY https://github.com/raws-labs/tigris-runtime.git
    GIT_TAG 08f5d2b0a69ae73d4e0ec2eb0ac22476300a9971)  # pinned runtime
