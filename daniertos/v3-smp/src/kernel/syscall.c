/**
 * danieRTOS v3.x - Kernel-side System Call Handler (SMP + User Mode)
 *
 * Copyright (c) 2025 Danny Jiang <djiang.tw@gmail.com>
 * SPDX-License-Identifier: MIT
 *
 * Implements the kernel-side syscall dispatcher for v3.x.
 * This is executed in M-mode when user tasks invoke ecall.
 */

#include "syscall.h"
#include "daniertos.h"
#include "smp.h"

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

reg_t *syscall_handler(reg_t *ctx)
{
    reg_t syscall_num = ctx[CTX_A7];
    reg_t arg0 = ctx[CTX_A0];
    reg_t arg1 = ctx[CTX_A1];
    reg_t arg2 = ctx[CTX_A2];
    reg_t ret = 0;

    /* Advance mepc past ecall instruction (4 bytes) */
    ctx[CTX_MEPC] += 4;

    switch (syscall_num) {
    /* -------------------------------------------------------------------------
     * Task Management
     * -------------------------------------------------------------------------*/
    case SYS_YIELD:
        return (reg_t *)sched_schedule(ctx);

    case SYS_DELAY:
        task_delay_internal((tick_t)arg0);
        return (reg_t *)sched_schedule(ctx);

    case SYS_GET_TICK:
        ret = (reg_t)tick_get();
        break;

    case SYS_EXIT:
        task_exit();
        return (reg_t *)sched_schedule(ctx);

    /* -------------------------------------------------------------------------
     * Semaphore Operations (v3p2)
     * -------------------------------------------------------------------------*/
    case SYS_SEM_WAIT:
        ret = sem_wait((semaphore_t *)arg0, (uint32_t)arg1) ? 1 : 0;
        /* sem_wait may block, need to reschedule */
        return (reg_t *)sched_schedule(ctx);

    case SYS_SEM_SIGNAL:
        sem_signal((semaphore_t *)arg0);
        /* May wake higher priority task, reschedule */
        return (reg_t *)sched_schedule(ctx);

    case SYS_SEM_TRYWAIT:
        ret = sem_try_wait((semaphore_t *)arg0) ? 1 : 0;
        break;

    /* -------------------------------------------------------------------------
     * Mutex Operations (v3p2)
     * -------------------------------------------------------------------------*/
    case SYS_MUTEX_LOCK:
        ret = mutex_lock((mutex_t *)arg0, (uint32_t)arg1) ? 1 : 0;
        /* mutex_lock may block, need to reschedule */
        return (reg_t *)sched_schedule(ctx);

    case SYS_MUTEX_UNLOCK:
        mutex_unlock((mutex_t *)arg0);
        /* May wake higher priority task, reschedule */
        return (reg_t *)sched_schedule(ctx);

    case SYS_MUTEX_TRYLOCK:
        ret = mutex_try_lock((mutex_t *)arg0) ? 1 : 0;
        break;

    /* -------------------------------------------------------------------------
     * Queue Operations (v3p2)
     * -------------------------------------------------------------------------*/
    case SYS_QUEUE_SEND:
        ret = queue_send((queue_t *)arg0, (const void *)arg1, (uint32_t)arg2) ? 1 : 0;
        /* queue_send may block or wake a receiver */
        return (reg_t *)sched_schedule(ctx);

    case SYS_QUEUE_RECV:
        ret = queue_receive((queue_t *)arg0, (void *)arg1, (uint32_t)arg2) ? 1 : 0;
        /* queue_receive may block or wake a sender */
        return (reg_t *)sched_schedule(ctx);

    /* -------------------------------------------------------------------------
     * I/O Operations
     * -------------------------------------------------------------------------*/
    case SYS_PUTCHAR:
        uart_putc((char)arg0);
        break;

    case SYS_PUTS:
        uart_puts((const char *)arg0);
        break;

    /* -------------------------------------------------------------------------
     * Debug/Info
     * -------------------------------------------------------------------------*/
    case SYS_GET_TASK_ID: {
        cpu_t *cpu = smp_get_cpu();
        ret = (reg_t)(cpu->current_task ? cpu->current_task->id : 0);
        break;
    }

    default:
        /* Unknown syscall - return error */
        ret = (reg_t)-1;
        break;
    }

    /* Set return value in a0 */
    ctx[CTX_A0] = ret;
    return ctx;
}

