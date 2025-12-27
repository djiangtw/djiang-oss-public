/**
 * danieRTOS v2.x - SMP Support
 *
 * Copyright (c) 2025 Danny Jiang <djiang.tw@gmail.com>
 * SPDX-License-Identifier: MIT
 */

#ifndef DANIERTOS_SMP_H
#define DANIERTOS_SMP_H

#include "types.h"
#include "config.h"

/* Forward declaration */
struct tcb;

/* -----------------------------------------------------------------------------
 * Per-Core Data Structure
 * ---------------------------------------------------------------------------*/

typedef struct cpu {
    uint64_t        hartid;         /* Hardware thread ID */
    struct tcb      *current_task;  /* Currently running task */
    struct tcb      *idle_task;     /* Idle task for this core */
    uint32_t        irq_nesting;    /* Interrupt nesting count */
    uint32_t        need_reschedule;/* Reschedule pending flag */
    uint8_t         padding[64 - 32]; /* Cache line padding */
} __attribute__((aligned(64))) cpu_t;

/* -----------------------------------------------------------------------------
 * Global Per-Core Array
 * Note: Uses CONFIG_MAX_CORES for static allocation to support runtime config
 * ---------------------------------------------------------------------------*/

extern cpu_t g_cpus[CONFIG_MAX_CORES];

/* -----------------------------------------------------------------------------
 * SMP API
 * ---------------------------------------------------------------------------*/

/**
 * Initialize SMP for BSP (Boot Strap Processor, core 0)
 */
void smp_init_bsp(void);

/**
 * Initialize SMP for AP (Application Processor, core 1+)
 */
void smp_init_ap(uint64_t hartid);

/**
 * Get current CPU structure (via tp register)
 */
static inline cpu_t *smp_get_cpu(void)
{
    cpu_t *cpu;
    asm volatile("mv %0, tp" : "=r"(cpu));
    return cpu;
}

/**
 * Get current hart ID
 */
static inline uint64_t smp_get_hartid(void)
{
    uint64_t hartid;
    asm volatile("csrr %0, mhartid" : "=r"(hartid));
    return hartid;
}

/**
 * Get number of cores
 */
static inline uint32_t smp_get_num_cores(void)
{
    return CONFIG_NUM_CORES;
}

/* -----------------------------------------------------------------------------
 * IPI (Inter-Processor Interrupt)
 * ---------------------------------------------------------------------------*/

/**
 * Send IPI to a specific core
 */
void smp_send_ipi(uint64_t target_hartid);

/**
 * Clear IPI for current core
 */
void smp_clear_ipi(void);

/**
 * Request reschedule on a specific core
 */
void smp_request_reschedule(uint64_t target_hartid);

/**
 * Handle IPI (called from trap handler)
 */
void smp_handle_ipi(void);

#endif /* DANIERTOS_SMP_H */

