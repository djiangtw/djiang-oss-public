/**
 * danieRTOS v3.x - SMP Support with User Mode Integration
 *
 * Copyright (c) 2025 Danny Jiang <djiang.tw@gmail.com>
 * SPDX-License-Identifier: MIT
 *
 * v3.x: Integrates SMP (from v2.x) with User Mode (from v1.x)
 *
 * Key Design: tp/mscratch Swap
 *   - User Mode:  mscratch = &cpu_t, tp = User TLS
 *   - Kernel Mode: tp = &cpu_t, mscratch = saved User tp
 *   - Trap entry swaps tp <-> mscratch, giving us cpu_t immediately
 *
 * cpu_t Structure Layout (CRITICAL - offsets used in assembly):
 *   +0:  kernel_trap_sp   (current task's kernel stack top)
 *   +8:  user_sp_save     (temporary save for user sp during trap)
 *   +16: user_tp_save     (saved user tp, restored on trap exit)
 *   +24: hartid
 *   ...
 */

#ifndef DANIERTOS_SMP_H
#define DANIERTOS_SMP_H

#include "types.h"
#include "config.h"

/* Forward declaration */
struct tcb;

/* -----------------------------------------------------------------------------
 * cpu_t Offsets for Assembly (MUST match struct cpu layout!)
 * ---------------------------------------------------------------------------*/

#define CPU_KERNEL_TRAP_SP  0
#define CPU_USER_SP_SAVE    8
#define CPU_USER_TP_SAVE    16
#define CPU_HARTID          24

/* -----------------------------------------------------------------------------
 * Per-Core Data Structure
 *
 * IMPORTANT: The first 3 fields are the "Trap Scratch Area" used by
 * trap_entry.S. Their order and offset MUST NOT change!
 * ---------------------------------------------------------------------------*/

typedef struct cpu {
    /* --- Trap Scratch Area (offsets used in assembly) --- */
    uint64_t        kernel_trap_sp; /* +0:  Kernel stack top for trap entry */
    uint64_t        user_sp_save;   /* +8:  Temp save for user SP */
    uint64_t        user_tp_save;   /* +16: Saved user TP (restored on mret) */

    /* --- Standard SMP Data --- */
    uint64_t        hartid;         /* +24: Hardware thread ID */
    struct tcb      *current_task;  /* Currently running task */
    struct tcb      *idle_task;     /* Idle task for this core */
    uint32_t        irq_nesting;    /* Interrupt nesting count */
    uint32_t        need_reschedule;/* Reschedule pending flag */

    /* --- Cache Line Padding --- */
    uint8_t         padding[64 - 56];
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
 *
 * NOTE: This only works in Kernel (M-mode). In User Mode, tp is User TLS.
 * The trap entry handler swaps tp/mscratch, so after trap entry tp = &cpu_t.
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

