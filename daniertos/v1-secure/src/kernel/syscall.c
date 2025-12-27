/**
 * danieRTOS v1.x - Kernel-side System Call Handler
 *
 * Copyright (c) 2025 Danny Jiang <djiang.tw@gmail.com>
 * SPDX-License-Identifier: MIT
 *
 * Implements the kernel-side syscall dispatcher.
 * User-mode wrappers are in src/user/user_syscall.c (linked to USER region).
 */

#include "syscall.h"
#include "daniertos.h"

/* Context frame register offsets (must match trap.S) */
#define CTX_A0      10
#define CTX_A1      11
#define CTX_A2      12
#define CTX_A7      17
#define CTX_MEPC    32

/* -----------------------------------------------------------------------------
 * Kernel-side syscall dispatcher
 *
 * Called from trap_handler when mcause indicates ecall from U-mode.
 * ---------------------------------------------------------------------------*/

reg_t *syscall_handler(reg_t *ctx) {
    reg_t syscall_num = ctx[CTX_A7];
    reg_t arg0 = ctx[CTX_A0];
    (void)ctx[CTX_A1];  /* arg1 reserved for future use */
    reg_t ret = 0;

    /* Advance mepc past ecall instruction (4 bytes) */
    ctx[CTX_MEPC] += 4;

    switch (syscall_num) {
    case SYS_YIELD:
        /* Trigger a reschedule */
        return (reg_t *)sched_schedule(ctx);

    case SYS_DELAY:
        task_delay((tick_t)arg0);
        return (reg_t *)sched_schedule(ctx);

    case SYS_GET_TICK:
        ret = (reg_t)tick_get();
        break;

    case SYS_EXIT:
        task_exit();
        return (reg_t *)sched_schedule(ctx);

    case SYS_PUTCHAR:
        uart_putc((char)arg0);
        break;

    case SYS_PUTS:
        uart_puts((const char *)arg0);
        break;

    case SYS_GET_TASK_ID:
        ret = (reg_t)task_get_current()->id;
        break;

    default:
        /* Unknown syscall - could panic or return error */
        ret = (reg_t)-1;
        break;
    }

    /* Set return value in a0 */
    ctx[CTX_A0] = ret;
    return ctx;
}

