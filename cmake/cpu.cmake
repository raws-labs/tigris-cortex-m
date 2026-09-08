# Map TIGRIS_TARGET_ARCH to GNU Arm Embedded per-core flags, mirroring TFLite
# Micro's cortex_m_generic target. Boards that own their toolchain (RP2350 via
# pico-sdk) set these through their SDK instead and do not call this.
function(tigris_cpu_flags arch out_var)
    if(arch STREQUAL "cortex-m4")
        set(flags -mcpu=cortex-m4 -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb)
    elseif(arch STREQUAL "cortex-m7")
        set(flags -mcpu=cortex-m7 -mfpu=fpv5-d16 -mfloat-abi=hard -mthumb)
    elseif(arch STREQUAL "cortex-m33")
        set(flags -mcpu=cortex-m33 -mfpu=fpv5-sp-d16 -mfloat-abi=hard -mthumb)
    else()
        message(FATAL_ERROR
            "Unknown TIGRIS_TARGET_ARCH '${arch}' "
            "(expected cortex-m4 | cortex-m7 | cortex-m33)")
    endif()
    set(${out_var} "${flags}" PARENT_SCOPE)
endfunction()
