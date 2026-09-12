# DS-CNN example

A pre-generated DS-CNN keyword-spotting model, so a board firmware builds with
only the ARM toolchain and no Python toolchain in the loop.

| File | What it is |
|------|------------|
| `ds_cnn_matched_64k.tgrs` | the compiled plan, 64 KiB fast budget |
| `model_blob.c` | the same plan as a C array, linked into flash |
| `tigris_codegen_core.c` / `.h` | the CMSIS-NN deployment core `tigris codegen` emitted for it |

Both the STM32 and the RP2350 board builds reference this directory, so a
firmware build needs no example-specific arguments:

```bash
cmake -B build -DTIGRIS_BOARD=nucleo_f446re
cmake --build build
```

To run a different model, regenerate all four files and replace them in place.
The [Cortex-M deployment
tutorial](https://tigris-ml.dev/tutorials/cortex-m-deployment/) walks that.
