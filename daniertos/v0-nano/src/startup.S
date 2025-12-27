/**
 * danieRTOS - Startup Code (RV64)
 *
 * Copyright (c) 2025 Danny Jiang <djiang.tw@gmail.com>
 * SPDX-License-Identifier: MIT
 */

.section .text.entry
.global _start
.type _start, @function

_start:
    /* -- Disable interrupts -- */
    csrw    mie, zero
    csrw    mip, zero

    /* -- Set up stack pointer -- */
    la      sp, _stack_top

    /* -- Set up global pointer (for relaxation) -- */
    .option push
    .option norelax
    la      gp, __global_pointer$
    .option pop

    /* -- Clear BSS -- */
    la      t0, _bss_start
    la      t1, _bss_end
1:
    bgeu    t0, t1, 2f
    sd      zero, 0(t0)
    addi    t0, t0, 8
    j       1b
2:

    /* -- Set trap vector -- */
    la      t0, trap_entry
    csrw    mtvec, t0

    /* -- Call main -- */
    call    main

    /* -- If main returns, hang -- */
hang:
    wfi
    j       hang

.size _start, . - _start

/* -----------------------------------------------------------------------------
 * Weak symbol for global pointer (linker will define if needed)
 * ---------------------------------------------------------------------------*/
.weak __global_pointer$

