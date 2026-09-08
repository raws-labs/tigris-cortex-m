/*
 * Host stub of the tigris-cortex-m HAL. Not a real target: it exists so the
 * portable app + HAL header can be compiled for a non-ARM host, proving no
 * board register or device header leaked into the portable layer (see
 * boards/host/CMakeLists.txt). The board runtime paths are proven on real
 * silicon; this is the fast, hardware-free CI check.
 */
#include <stdint.h>
#include <stdio.h>
#include <time.h>

#include "tigris_hal.h"

void tigris_hal_init(void) {}
uint32_t tigris_hal_cpu_hz(void) { return 1000000000u; }
uint32_t tigris_hal_cycles(void) { return (uint32_t)clock(); }
void tigris_hal_putc(char c) { putchar(c); }
const char *tigris_hal_board_name(void) { return "host"; }
const volatile uint32_t *tigris_hal_clock_diag(void) { return NULL; }
void tigris_hal_halt(void) { fflush(stdout); }
