# Pinned upstream dependencies, fetched by exact commit.
#
# For an offline / local-mirror build, override any source with
#   -DFETCHCONTENT_SOURCE_DIR_<NAME>=<path>
# where NAME is upper-cased: CMSIS_NN, CMSIS_CORE, TIGRIS_RUNTIME.
include(FetchContent)

# CMSIS-NN and CMSIS-Core are fetched as SOURCES ONLY. SOURCE_SUBDIR points at a
# nonexistent directory so FetchContent_MakeAvailable populates the trees without
# add_subdirectory'ing their own CMake projects; cmake/cmsis_nn.cmake adds
# CMSIS-NN's project itself, with the optimization level and include settings
# the runtime needs.
FetchContent_Declare(cmsis_nn
    GIT_REPOSITORY https://github.com/ARM-software/CMSIS-NN.git
    GIT_TAG 13c97dbb6f781d4aab38ed34e6e441f42b79aff4  # v8.0.0
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
    GIT_TAG 65c4d6290a1959c7f956e0b492a8997dd20fda06)  # pinned runtime
