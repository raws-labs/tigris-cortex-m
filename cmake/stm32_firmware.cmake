# Shared firmware-target definition for a vendored-CMSIS-Device STM32 board.
# The cmake/boards/nucleo_*.cmake files call this with their board specifics;
# the CPU flags, -O2 and toolchain are set by the top-level CMakeLists. The
# CMSIS-Device pack (startup + system + device header) is fetched source-only.
function(tigris_add_stm32_firmware)
    cmake_parse_arguments(FW ""
        "DEVICE_NAME;DEVICE_REPO;DEVICE_TAG;DEFINE;STARTUP;SYSTEM;PLATFORM;LINKER;FAST;SLOW"
        "" ${ARGN})

    add_compile_definitions(${FW_DEFINE})

    FetchContent_Declare(${FW_DEVICE_NAME}
        GIT_REPOSITORY ${FW_DEVICE_REPO}
        GIT_TAG ${FW_DEVICE_TAG}
        SOURCE_SUBDIR __tigris_no_add_subdir__)
    FetchContent_MakeAvailable(${FW_DEVICE_NAME})
    set(_dev ${${FW_DEVICE_NAME}_SOURCE_DIR})

    set(_plat    ${TIGRIS_CM_ROOT}/src/hal/platform)
    set(_example ${TIGRIS_CM_ROOT}/examples/ds_cnn)

    add_executable(tigris_firmware
        ${TIGRIS_CM_ROOT}/src/app/tigris_app.c
        ${_plat}/${FW_PLATFORM}
        ${_example}/tigris_codegen_core.c
        ${_example}/model_blob.c
        ${_dev}/${FW_SYSTEM}
        ${_dev}/${FW_STARTUP})

    target_include_directories(tigris_firmware PRIVATE
        ${TIGRIS_CM_ROOT}/include
        ${_example}
        ${cmsis_core_SOURCE_DIR}/CMSIS/Core/Include
        ${_dev}/Include)

    target_compile_definitions(tigris_firmware PRIVATE
        TIGRIS_APP_FAST_ARENA_BYTES=${FW_FAST}
        TIGRIS_APP_SLOW_ARENA_BYTES=${FW_SLOW})

    target_link_libraries(tigris_firmware PRIVATE tigris_runtime cmsis-nn)

    target_link_options(tigris_firmware PRIVATE
        ${TIGRIS_CPU_FLAGS}          # so the gcc link driver selects the hard-float multilib
        -T${_plat}/${FW_LINKER}
        --specs=nano.specs
        -u _printf_float
        -Wl,--gc-sections
        -Wl,-Map=${CMAKE_BINARY_DIR}/tigris_firmware.map)

    add_custom_command(TARGET tigris_firmware POST_BUILD
        COMMAND ${CMAKE_OBJCOPY} -O binary
                $<TARGET_FILE:tigris_firmware>
                ${CMAKE_BINARY_DIR}/tigris_firmware.bin
        COMMAND ${CMAKE_SIZE} $<TARGET_FILE:tigris_firmware>
        COMMENT "Creating tigris_firmware.bin")
endfunction()
