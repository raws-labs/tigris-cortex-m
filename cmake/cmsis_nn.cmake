# The `cmsis-nn` static library the tigris runtime links when TIGRIS_HAS_CMSIS_NN
# is ON. The runtime references this target but deliberately does not define it
# (see tigris-runtime CMakeLists), so the target-support package supplies it.
# CMSIS-NN's own CMake project defines the target and selects its sources (the
# float kernels stay out unless ARM_NN_ENABLE_F32 / ARM_NN_ENABLE_F16 is set);
# sources come from the FetchContent'd CMSIS-NN + CMSIS-Core in cmake/deps.cmake.
if(NOT EXISTS ${cmsis_nn_SOURCE_DIR}/Include/arm_nnfunctions.h)
    message(FATAL_ERROR "CMSIS-NN not populated at ${cmsis_nn_SOURCE_DIR}")
endif()

# -O2 is the measured sweet spot on Cortex-M7: both -O3 and -Ofast were ~2.5%
# SLOWER on-device (extra inlining bloats .text and evicts the hot kernels from
# the 16 KB I-cache; the penalty is in the cycle count, not a wait-state, and
# holds at 64 and 480 MHz). Override CMSIS_NN_OPT to re-measure. The per-core
# flags (-mcpu/-mfpu/...) are inherited from the parent's add_compile_options,
# so the DSP paths (__ARM_FEATURE_DSP) auto-select.
set(CMSIS_NN_OPT "-O2" CACHE STRING "optimization level for the CMSIS-NN kernels")
set(CMSIS_OPTIMIZATION_LEVEL ${CMSIS_NN_OPT} CACHE STRING "" FORCE)
# CMSIS-NN adds ${CMSIS_PATH}/CMSIS/Core/Include to the target's include path.
set(CMSIS_PATH ${cmsis_core_SOURCE_DIR})
# SYSTEM, so the runtime's -Wpedantic -Werror does not trip on vendor headers.
add_subdirectory(${cmsis_nn_SOURCE_DIR} ${cmsis_nn_BINARY_DIR} SYSTEM)
target_compile_options(cmsis-nn PRIVATE -w)
