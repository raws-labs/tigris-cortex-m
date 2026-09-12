# tigris-cortex-m

Board bring-up and CMSIS-NN link glue for running TiGrIS plans on Cortex-M: NUCLEO-F446RE (M4F), NUCLEO-H753ZI (M7), Raspberry Pi Pico 2 (RP2350, M33). CMake with arm-none-eabi; Pico 2 through pico-sdk. Public, github.com/raws-labs/tigris-cortex-m.

## Build, test, run
- STM32: `cmake -B build -DTIGRIS_BOARD=nucleo_f446re` (or `nucleo_h753zi`) `&& cmake --build build`; output `build/tigris_firmware.bin`, flash at 0x08000000. `cmake/toolchains/arm-none-eabi.cmake` is selected automatically.
- Pico 2: `PICO_SDK_PATH=<sdk> cmake -S boards/pico2 -B build -Dpicotool_DIR=<picotool>/lib/cmake/picotool && cmake --build build`; output `build/tigris_firmware.uf2`. `-DTIGRIS_BOARD=pico2` at the top level fails on purpose.
- No board: `cmake -B build-spine && cmake --build build-spine` builds the runtime plus `cmsis-nn` for `TIGRIS_TARGET_ARCH` (default cortex-m4).
- Host guard, no hardware: `cmake -S boards/host -B build-host && cmake --build build-host` compiles the portable app, HAL header and example natively so board registers leaking into portable code fail fast.
- The firmware runs one inference of the bundled DS-CNN (`examples/ds_cnn/`), prints the int8 output and cycle count over ST-LINK VCP (STM32) or USB-CDC (RP2350), then halts.

## Layout
- `cmake/deps.cmake`: FetchContent pins by exact commit for CMSIS-NN, CMSIS-Core and tigris-runtime (the pins are the only place versions live); `cmake/cmsis_nn.cmake` builds the `cmsis-nn` target the runtime links (`TIGRIS_HAS_CMSIS_NN` forced ON). Offline: `-DFETCHCONTENT_SOURCE_DIR_<CMSIS_NN|CMSIS_CORE|TIGRIS_RUNTIME|CMSIS_DEVICE_F4|CMSIS_DEVICE_H7>=<path>`.
- `cmake/boards/<board>.cmake` calls `tigris_add_stm32_firmware()` (`cmake/stm32_firmware.cmake`) with device pack, startup, linker and arena sizes; `src/hal/platform/<board>/` implements `include/tigris_hal.h` (clock, printf UART, cycle counter, halt). Porting a board means one platform dir, one linker script, one board file.
- `src/app/tigris_app.c`: portable load/run/print flow; arenas are `TIGRIS_APP_FAST_ARENA_BYTES` / `TIGRIS_APP_SLOW_ARENA_BYTES` (defaults 48 KB / 16 KB; board files set 72 KB / 16 KB).
- `tools/bin2c.py model.tgrs model_blob.c --symbol g_tigris_plan`: embeds a plan as a 16-byte-aligned `.rodata` array for the zero-copy loader.

## Gotchas
- The runtime commit is pinned in three files that must move together: `cmake/deps.cmake`, `boards/pico2/CMakeLists.txt`, `boards/host/CMakeLists.txt`.
- The CMSIS-NN fast arena is provisioned at the plan's `-m` budget, not at the smaller activation peak, so the budget itself must fit the board SRAM.
- `-O2` is the measured optimum for CMSIS-NN on Cortex-M7; `-O3` and `-Ofast` are slower (I-cache eviction). `CMSIS_NN_OPT` overrides it; the pico2 project forces `-O2` before `pico_sdk_init` because pico-sdk defaults Release to `-O3`.
- H753: arenas and `.bss` live in AXI SRAM (512 KB), stack in DTCM (`stm32h753zi.ld`); clock_init targets 480 MHz off PLL1 with an HSI 64 MHz fallback, and `tigris_hal_clock_diag()` reports which was reached.
- Pico 2 compiles the runtime and CMSIS-NN sources directly into the firmware (pico-sdk convention) instead of linking the runtime library.
