/* Auto-generated-code API.  This header is target-neutral. */
#ifndef TIGRIS_CODEGEN_CORE_H
#define TIGRIS_CODEGEN_CORE_H

#include <stdint.h>

#include "tigris.h"
#include "tigris_executor.h"
#include "tigris_loader.h"
#include "tigris_mem.h"

/* Model-specific compile-time requirements for static embedding. The plan
 * cost model uses 32-byte allocations, conservatively covering supported
 * runtimes whose TIGRIS_TENSOR_ALIGN is at most this value. Backend-specific
 * scratch/workspace is prepared separately and is not included here. */
#define TIGRIS_CODEGEN_TENSOR_CAPACITY 13u
#define TIGRIS_CODEGEN_PLAN_TENSOR_ALIGNMENT_BYTES 32u
#define TIGRIS_CODEGEN_PLAN_BUDGET_BYTES 65536u
#define TIGRIS_CODEGEN_WEIGHT_DECOMPRESSION_RESERVE_BYTES 0u
#define TIGRIS_CODEGEN_CORE_FAST_ARENA_BYTES 65536u
#define TIGRIS_CODEGEN_EXECUTOR_WORKSPACE_BYTES \
    TIGRIS_EXECUTOR_WORKSPACE_BYTES_FOR_LIMITS(13u, 1u, 1u, 0u, 0u)

typedef void (*tigris_codegen_input_init_fn)(
    void *data, uint32_t size_bytes, uint16_t tensor_index, void *user_ctx);

/* Load and validate the serialized plan supplied by the embedding app. */
tigris_error_t tigris_codegen_load_plan(
    const uint8_t *plan_data, uint32_t plan_len, tigris_plan_t *out_plan);

/* Set up runtime memory, prepare the selected backend, and allocate inputs.
 * Call once before using reset/run. ``init_input`` may be NULL. */
tigris_mem_error_t tigris_codegen_init(
    const tigris_plan_t *plan, tigris_mem_t *mem,
    void **tensor_ptrs, uint16_t tensor_capacity,
    void *fast_arena, uint32_t fast_arena_size,
    void *slow_arena, uint32_t slow_arena_size,
    tigris_codegen_input_init_fn init_input, void *user_ctx);

/* Reset activations and inputs for another inference. The backend reservation
 * created by init is retained, so CMSIS-NN scratch cannot alias activations. */
tigris_mem_error_t tigris_codegen_reset(
    const tigris_plan_t *plan, tigris_mem_t *mem,
    tigris_codegen_input_init_fn init_input, void *user_ctx);

/* The backend dispatcher is generated from --backend. */
tigris_kernel_fn tigris_codegen_dispatch(void);

/* Run using the generated dispatcher and caller-owned executor workspace. */
tigris_exec_error_t tigris_codegen_run(
    const tigris_plan_t *plan, tigris_mem_t *mem, tigris_exec_stats_t *stats,
    tigris_executor_workspace_t *workspace);

/* Plan-sized alternative: no generic executor limits are reserved. */
tigris_exec_error_t tigris_codegen_run_with_workspace_buffer(
    const tigris_plan_t *plan, tigris_mem_t *mem, tigris_exec_stats_t *stats,
    void *workspace, size_t workspace_size);

#endif
