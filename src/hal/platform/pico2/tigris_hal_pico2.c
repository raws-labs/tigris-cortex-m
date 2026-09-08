/*
 * tigris-cortex-m board HAL for Raspberry Pi Pico 2 (RP2350, Cortex-M33), on
 * top of the pico-sdk. Unlike the STM32 boards (hand-written CMSIS BSPs), the
 * RP2350 has no on-board VCP, so the SDK provides clock setup, the flash/XIP
 * boot, and stdio over USB-CDC (printf is retargeted to it).
 *
 * Timing uses the RP2350 hardware microsecond timer (time_us), NOT the Armv8-M
 * DWT cycle counter: the DWT CYCCNT on RP2350 under-counts (it disagrees with
 * the wall clock for memory-heavy code, apparently not counting XIP stalls).
 * time_us is the trustworthy wall clock; we report clock-equivalent "cycles" =
 * us * MHz, so cycles / cpu_hz gives the real time. The app prints once and
 * halts, so init waits (bounded) for the USB-CDC host before running.
 */
#include <stdint.h>
#include <stdio.h>

#include "pico/stdlib.h"
#include "hardware/clocks.h"
#include "pico/time.h"

#include "tigris_hal.h"

static uint32_t s_mhz = 150;
static volatile uint32_t s_clk_diag[8];

const volatile uint32_t *tigris_hal_clock_diag(void) { return s_clk_diag; }

void tigris_hal_init(void)
{
    /* RP2350 default sys clock is 150 MHz; set it explicitly for clarity. */
    set_sys_clock_khz(150000, true);
    s_mhz = (uint32_t)(clock_get_hz(clk_sys) / 1000000u);

    stdio_init_all();   /* USB-CDC; printf retargets here */
    /* Wait (bounded ~6 s) for the host to open the CDC port before running,
     * otherwise the one-shot output is lost. */
    for (int i = 0; i < 600 && !stdio_usb_connected(); i++)
        sleep_ms(10);
    sleep_ms(150);

    s_clk_diag[0] = 5;                              /* stage 5 = clock up */
    s_clk_diag[1] = (uint32_t)clock_get_hz(clk_sys);
}

uint32_t tigris_hal_cpu_hz(void) { return (uint32_t)clock_get_hz(clk_sys); }

/* Wall-clock cycles = elapsed microseconds * MHz (see header note on the DWT). */
uint32_t tigris_hal_cycles(void) { return (uint32_t)(time_us_64() * (uint64_t)s_mhz); }

void tigris_hal_putc(char c) { putchar(c); }

const char *tigris_hal_board_name(void) { return "pico2_rp2350"; }

void tigris_hal_halt(void)
{
    for (;;)
        tight_loop_contents();
}
