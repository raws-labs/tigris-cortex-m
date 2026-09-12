# tigris-cortex-m

Target support for deploying [TiGrIS](https://tigris-ml.dev)-compiled models on
ARM Cortex-M microcontrollers with CMSIS-NN. It provides board bring-up and
build glue around the portable
[tigris-runtime](https://github.com/raws-labs/tigris-runtime) core: the runtime
carries the CMSIS-NN kernel adapter, and this package supplies the `cmsis-nn`
library it links, each board's clock / serial / linker, and a minimal example app.

## Boards

| Board | MCU | Core | Build |
|-------|-----|------|-------|
| NUCLEO-F446RE | STM32F446RE | Cortex-M4F | CMake + arm-none-eabi |
| NUCLEO-H753ZI | STM32H753ZI | Cortex-M7 | CMake + arm-none-eabi |
| Raspberry Pi Pico 2 | RP2350 | Cortex-M33 | pico-sdk |

## Build and flash the example

The bundled DS-CNN example (`examples/ds_cnn/`) is pre-generated, so a board
firmware builds with only the ARM toolchain.

STM32 (F446 / H753):

```bash
cmake -B build -DTIGRIS_BOARD=nucleo_f446re      # or nucleo_h753zi
cmake --build build
# -> build/tigris_firmware.bin ; flash at 0x08000000 (st-flash / ST-LINK / OpenOCD)
```

RP2350 (needs `PICO_SDK_PATH` and picotool):

```bash
PICO_SDK_PATH=/path/to/pico-sdk cmake -S boards/pico2 -B build \
  -Dpicotool_DIR=/path/to/picotool/install/lib/cmake/picotool
cmake --build build
# -> build/tigris_firmware.uf2 ; BOOTSEL drag-and-drop or `picotool load`
```

The firmware runs one inference on a fixed input and prints the int8 output and
cycle count over serial (STM32 ST-LINK VCP) or USB-CDC (RP2350).

## Deploy your own model

Compile a plan, generate a CMSIS-NN deployment core, embed the plan with
`tools/bin2c.py`, then drop the three generated files in place of the ones in
`examples/ds_cnn/` and size the arenas with `-DTIGRIS_APP_FAST_ARENA_BYTES=` /
`-DTIGRIS_APP_SLOW_ARENA_BYTES=`. The [Cortex-M deployment
tutorial](https://tigris-ml.dev/tutorials/cortex-m-deployment/) walks the whole
sequence with commands.

The plan's `-m` budget must fit the board's SRAM: the CMSIS-NN fast arena is
provisioned at the budget, not at the (smaller) activation peak.

## Porting a board

Implement `include/tigris_hal.h` (clock, a UART that printf retargets to, a cycle
counter) under `src/hal/platform/<board>/`, add a linker script, and add a board
file: copy `cmake/boards/nucleo_f446re.cmake` for a vendored-CMSIS STM32 part, or
`boards/pico2/CMakeLists.txt` for a pico-sdk target. See `src/app/tigris_app.c`
for the load / run / output flow the HAL supports.

## Dependencies

Fetched by pinned commit (CMake FetchContent): the tigris runtime, ARM CMSIS-NN
+ CMSIS-Core, and the STM32 CMSIS-Device pack (RP2350 pulls its bring-up from
pico-sdk). For an offline build, point any source at a local mirror with
`-DFETCHCONTENT_SOURCE_DIR_<NAME>=<path>` (`CMSIS_NN`, `CMSIS_CORE`,
`TIGRIS_RUNTIME`, `CMSIS_DEVICE_F4`, `CMSIS_DEVICE_H7`).

A hardware-free host compile check lives at `boards/host/`:
`cmake -S boards/host -B build-host && cmake --build build-host`.
