/*
 * tigris-cortex-m board HAL.
 *
 * The portable app (src/app/tigris_app.c) talks to the hardware only through
 * this interface. Each board under src/hal/platform/<board>/ implements it:
 * clock bring-up, a UART that stdio printf is retargeted to, and a cycle
 * counter for timing. No board header or register access belongs above this
 * line.
 */
#ifndef TIGRIS_HAL_H
#define TIGRIS_HAL_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Bring up clocks, the result UART (printf is retargeted to it), the cycle
 * counter, and any caches. Call once at the top of main(). */
void tigris_hal_init(void);

/* Core clock in Hz, to convert cycle counts to wall-clock time. */
uint32_t tigris_hal_cpu_hz(void);

/* Free-running monotonic cycle/us counter (DWT CYCCNT on STM32; time_us on
 * RP2350). 32-bit; wraps far beyond one inference. */
uint32_t tigris_hal_cycles(void);

/* Write one byte to the result UART. printf is retargeted here via _write. */
void tigris_hal_putc(char c);

/* Short board name for the results line (e.g. "nucleo_f446re"). */
const char *tigris_hal_board_name(void);

/* Optional bring-up diagnostics: an 8-word snapshot of the clock/power state
 * captured at the end of clock setup, or NULL if unsupported. Lets a run
 * confirm the target clock was reached without a debugger. */
const volatile uint32_t *tigris_hal_clock_diag(void);

/* Disable interrupts and spin forever, so the final serial output is not
 * disturbed by a reset loop. Never returns. */
void tigris_hal_halt(void);

#ifdef __cplusplus
}
#endif

#endif /* TIGRIS_HAL_H */
