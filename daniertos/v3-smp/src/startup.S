/**
 * danieRTOS v3.x - Startup Code (RV64 SMP + User Mode)
 *
 * Copyright (c) 2025 Danny Jiang <djiang.tw@gmail.com>
 * SPDX-License-Identifier: MIT
 *
 * v3.x = v2.x (SMP) + v1.x (User Mode)
 *
 * Key additions from v1.x:
 *   - PMP initialization for memory protection
 *   - mscratch setup for tp/mscratch swap in trap handler
 *
 * Multi-core boot sequence:
 * 1. All harts start at _start simultaneously
 * 2. Hart 0 (BSP) initializes BSS, PMP, trap, kernel
 * 3. Hart 1+ (AP) busy-wait on g_boot_flag
 * 4. BSP sets g_boot_flag = 1 when kernel is ready
 * 5. APs see flag, initialize their PMP, trap vector, call main_secondary
 * 6. Each hart has its own stack: sp = _stack_top - hartid * STACK_SIZE
 *
 * tp/mscratch initialization:
 *   - tp is set to &g_cpus[hartid] after smp_init
 *   - mscratch is initially 0, set properly when entering U-mode
 */

#include "config.h"

.equ STACK_SIZE, 0x10000    /* 64 KB per core */

.section .text.entry
.global _start
.type _start, @function

_start:
    /* -- Disable interrupts -- */
    csrw    mie, zero
    csrw    mip, zero

    /* -- Get hart ID -- */
    csrr    t0, mhartid

    /* -- Set up per-core stack -- */
    /* sp = _stack_top - (hartid * STACK_SIZE) */
    la      sp, _stack_top
    li      t1, STACK_SIZE
    mul     t1, t1, t0
    sub     sp, sp, t1

    /* -- Set up global pointer (for relaxation) -- */
    .option push
    .option norelax
    la      gp, __global_pointer$
    .option pop

    /* -- Branch based on hart ID -- */
    bnez    t0, _start_ap

/* -----------------------------------------------------------------------------
 * BSP (Boot Strap Processor) - Hart 0
 * ---------------------------------------------------------------------------*/
_start_bsp:
    /* -- Clear BSS -- */
    la      t0, _bss_start
    la      t1, _bss_end
1:
    bgeu    t0, t1, 2f
    sd      zero, 0(t0)
    addi    t0, t0, 8
    j       1b
2:

#if CONFIG_USER_MODE
    /* -- Initialize mscratch to zero --
     * Will be set to &cpu_t when entering U-mode.
     * In Kernel mode, tp = &cpu_t and mscratch holds saved User tp. */
    csrw    mscratch, zero

    /* -- Initialize PMP for memory protection --
     * Each hart must configure its own PMP (PMP is per-hart). */
    call    pmp_init
#endif

    /* -- Set trap vector -- */
    la      t0, trap_entry
    csrw    mtvec, t0

    /* -- Enable software interrupt (for IPI) -- */
    li      t0, 0x8         /* MIE_MSIE */
    csrs    mie, t0

    /* -- Call main (BSP) -- */
    /* APs are in WFI, BSP will wake them via IPI after kernel init */
    call    main

    /* -- If main returns, hang -- */
    j       hang

/* -----------------------------------------------------------------------------
 * AP (Application Processor) - Hart 1+
 * ---------------------------------------------------------------------------*/
_start_ap:
    /* -- Wait for BSP to complete kernel initialization -- */
    /* Pure busy-wait polling on shared memory flag */
    la      t1, g_boot_flag
_ap_wait:
    fence   r, r            /* Ensure we see latest value */
    lw      t2, 0(t1)
    beqz    t2, _ap_wait    /* Spin while flag is 0 */
    fence   r, rw           /* Ensure we see all kernel data */

#if CONFIG_USER_MODE
    /* -- Initialize mscratch to zero -- */
    csrw    mscratch, zero

    /* -- Initialize PMP for this hart --
     * PMP is per-hart, so each AP must configure its own. */
    call    pmp_init
#endif

    /* -- Set trap vector -- */
    la      t0, trap_entry
    csrw    mtvec, t0

    /* -- Enable software interrupt (for IPI) -- */
    li      t0, 0x8         /* MIE_MSIE */
    csrs    mie, t0

    /* -- Call secondary main -- */
    csrr    a0, mhartid     /* Pass hartid as argument */
    call    main_secondary

    /* Fall through to hang */

hang:
    wfi
    j       hang

.size _start, . - _start

/* -----------------------------------------------------------------------------
 * Boot Flag (BSP sets to 1 when APs should start)
 * ---------------------------------------------------------------------------*/
.section .data
.global g_boot_flag
g_boot_flag:
    .word   0

/* -----------------------------------------------------------------------------
 * Weak symbols
 * ---------------------------------------------------------------------------*/
.weak __global_pointer$

/* Weak PMP init (in case CONFIG_USER_MODE is disabled) */
.weak pmp_init
pmp_init:
    ret

