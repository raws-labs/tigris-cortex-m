/*
 * Minimal TiGrIS deployment app for Cortex-M.
 *
 * Loads the model plan embedded in flash (g_tigris_plan, produced by bin2c),
 * runs one int8 inference through the generated CMSIS-NN core, prints the
 * output vector and the measured cycle count, then halts. This is the "run
 * once" analog of the runtime's examples/posix/main.c, for a bare-metal board.
 * It talks to the hardware only through tigris_hal.h.
 */
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "tigris.h"
#include "tigris_executor.h"
#include "tigris_mem.h"
#include "tigris_kernels_cmsis_nn.h"

#include "tigris_codegen_core.h"
#include "tigris_hal.h"

extern const unsigned char g_tigris_plan[];
extern const unsigned int  g_tigris_plan_len;

/* Static arena backing store. Sized to fit the board's SRAM, NOT the plan's
 * compile budget (the codegen budget can far exceed a small board's SRAM; the
 * runtime only needs tigris_cmsis_nn_fast_arena_required() at run time, e.g.
 * ~17 KB for DS-CNN). Override per board/model with -DTIGRIS_APP_FAST_ARENA_BYTES
 * / -DTIGRIS_APP_SLOW_ARENA_BYTES. Defaults fit a 128 KB single-SRAM board. */
#ifndef TIGRIS_APP_FAST_ARENA_BYTES
#define TIGRIS_APP_FAST_ARENA_BYTES (48u * 1024u)
#endif
#ifndef TIGRIS_APP_SLOW_ARENA_BYTES
#define TIGRIS_APP_SLOW_ARENA_BYTES (16u * 1024u)
#endif

static uint8_t s_fast_arena[TIGRIS_APP_FAST_ARENA_BYTES]
    __attribute__((aligned(16)));
static uint8_t s_slow_arena[TIGRIS_APP_SLOW_ARENA_BYTES]
    __attribute__((aligned(16)));
static void   *s_tensor_ptrs[TIGRIS_CODEGEN_TENSOR_CAPACITY];
static uint8_t s_workspace[TIGRIS_CODEGEN_EXECUTOR_WORKSPACE_BYTES];

/* Deterministic reference input: every input byte = 1, matching the host
 * reference so the device output is reproducible and diffable. */
static void fill_input(void *data, uint32_t size_bytes,
                       uint16_t tensor_index, void *user_ctx)
{
    (void)tensor_index;
    (void)user_ctx;
    memset(data, 1, size_bytes);
}

int main(void)
{
    tigris_hal_init();
    printf("Board:   %s\n", tigris_hal_board_name());
    printf("Clock:   %lu Hz\n", (unsigned long)tigris_hal_cpu_hz());

    tigris_plan_t plan;
    tigris_error_t perr =
        tigris_codegen_load_plan(g_tigris_plan, g_tigris_plan_len, &plan);
    if (perr != TIGRIS_OK) {
        printf("plan load failed: %s\n", tigris_error_str(perr));
        tigris_hal_halt();
    }
    printf("Model:   %s\n", tigris_model_name(&plan));
    printf("Ops:     %u\n", plan.header->num_ops);
    printf("Stages:  %u\n", plan.header->num_stages);

    uint32_t fast_size = tigris_cmsis_nn_fast_arena_required(&plan);
    if (fast_size == UINT32_MAX || fast_size > sizeof(s_fast_arena)) {
        printf("fast arena too small: need %lu, have %lu\n",
               (unsigned long)fast_size, (unsigned long)sizeof(s_fast_arena));
        tigris_hal_halt();
    }

    tigris_mem_t mem;
    tigris_mem_error_t merr = tigris_codegen_init(
        &plan, &mem, s_tensor_ptrs, TIGRIS_CODEGEN_TENSOR_CAPACITY,
        s_fast_arena, fast_size, s_slow_arena, sizeof(s_slow_arena),
        fill_input, NULL);
    if (merr != TIGRIS_MEM_OK) {
        printf("mem init failed: %s\n", tigris_mem_error_str(merr));
        tigris_hal_halt();
    }

    tigris_exec_stats_t stats;
    uint32_t c0 = tigris_hal_cycles();
    tigris_exec_error_t eerr = tigris_codegen_run_with_workspace_buffer(
        &plan, &mem, &stats, s_workspace, sizeof(s_workspace));
    uint32_t c1 = tigris_hal_cycles();
    if (eerr != TIGRIS_EXEC_OK) {
        printf("inference failed: %s\n", tigris_exec_error_str(eerr));
        tigris_hal_halt();
    }

    /* Output tensors as int8. Stale plans (compiled before the
     * num_model_outputs fix) report 0; fall back to the last op's first
     * output, as the ESP harness does. */
    uint16_t n_out = plan.header->num_model_outputs;
    uint16_t fallback_out;
    const uint16_t *out_list;
    if (n_out > 0) {
        out_list = plan.model_outputs;
    } else {
        const tigris_op_t *last = &plan.ops[plan.header->num_ops - 1];
        fallback_out = tigris_op_outputs(&plan, last)[0];
        out_list = &fallback_out;
        n_out = 1;
    }
    for (uint16_t i = 0; i < n_out; i++) {
        uint16_t tidx = out_list[i];
        const int8_t *out = (const int8_t *)mem.tensor_ptrs[tidx];
        if (!out)
            continue;
        uint32_t n = plan.tensors[tidx].size_bytes;
        printf("OUTPUT_I8:");
        for (uint32_t j = 0; j < n; j++)
            printf(" %d", (int)out[j]);
        printf("\n");
    }

    printf("Cycles:  %lu\n", (unsigned long)(c1 - c0));
    printf("DONE\n");
    tigris_hal_halt();
    return 0;
}
