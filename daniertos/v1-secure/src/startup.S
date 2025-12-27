/**
 * danieRTOS v1.x - Startup Code (RV64)
 *
 * Copyright (c) 2025 Danny Jiang <djiang.tw@gmail.com>
 * SPDX-License-Identifier: MIT
 *
 * v1.x: Added PMP initialization and mscratch setup for M+U mode
 */

#include "config.h"

.section .text.entry
.global _start
.type _start, @function

_start:
    /* ======================================================================
     * Phase 1: Basic initialization (same as v0)
     * ======================================================================*/

    /* -- Disable all interrupts -- */
    csrw    mie, zero
    csrw    mip, zero

    /* -- Set up bootstrap stack pointer -- */
    la      sp, _stack_top

    /* -- Set up global pointer (for relaxation) -- */
    .option push
    .option norelax
    la      gp, __global_pointer$
    .option pop

    /* -- Clear all BSS sections -- */
    /* Kernel BSS */
    la      t0, _kernel_bss_start
    la      t1, _kernel_bss_end
1:
    bgeu    t0, t1, 2f
    sd      zero, 0(t0)
    addi    t0, t0, 8
    j       1b
2:
    /* User BSS */
    la      t0, _user_bss_start
    la      t1, _user_bss_end
3:
    bgeu    t0, t1, 4f
    sd      zero, 0(t0)
    addi    t0, t0, 8
    j       3b
4:

    /* ======================================================================
     * Phase 2: M+U mode initialization (v1.x specific)
     * ======================================================================*/

#if CONFIG_USER_MODE
    /* -- Initialize mscratch to zero (will be set per-task) -- */
    csrw    mscratch, zero

    /* -- Configure PMP for memory protection --
     *
     * PMP Entry 0: Kernel region (0x80000000 - 0x8001FFFF) - M-mode only
     * PMP Entry 1: User region   (0x80020000 - 0x8007FFFF) - U-mode RWX
     * PMP Entry 2: Shared region (0x80080000 - 0x8008FFFF) - U-mode R
     * PMP Entry 3: Stack region  (0x80090000 - 0x800FFFFF) - U-mode RW
     * PMP Entry 4: CLINT         (0x02000000) - M-mode only
     * PMP Entry 5: PLIC          (0x0C000000) - M-mode only
     * PMP Entry 6: UART          (0x10000000) - M-mode only
     * PMP Entry 7: Catch-all     (all remaining) - M-mode only
     *
     * Note: PMP entries are checked in order; first match wins.
     * We use NAPOT (Naturally Aligned Power-of-Two) addressing.
     */

    /* Call C function for complex PMP setup */
    call    pmp_init
#endif

    /* -- Set trap vector -- */
    la      t0, trap_entry
    csrw    mtvec, t0

    /* ======================================================================
     * Phase 3: Jump to main
     * ======================================================================*/

    /* -- Call main -- */
    call    main

    /* -- If main returns, hang -- */
hang:
    wfi
    j       hang

.size _start, . - _start

/* -----------------------------------------------------------------------------
 * Weak symbols
 * ---------------------------------------------------------------------------*/
.weak __global_pointer$

/* Weak PMP init (in case CONFIG_USER_MODE is disabled) */
.weak pmp_init
pmp_init:
    ret

