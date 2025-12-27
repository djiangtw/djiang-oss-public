/**
 * danieRTOS v1.x - System Call Interface
 *
 * Copyright (c) 2025 Danny Jiang <djiang.tw@gmail.com>
 * SPDX-License-Identifier: MIT
 *
 * Defines the syscall numbers and user-mode API wrappers.
 * User tasks call these functions which trigger ecall to enter M-mode.
 */

#ifndef DANIERTOS_SYSCALL_H
#define DANIERTOS_SYSCALL_H

#include "types.h"

/* -----------------------------------------------------------------------------
 * System Call Numbers
 *
 * Convention: a7 = syscall number, a0-a5 = arguments, a0 = return value
 * ---------------------------------------------------------------------------*/

/* Task management */
#define SYS_YIELD           0   /* Yield CPU to other tasks */
#define SYS_DELAY           1   /* Delay for specified ticks */
#define SYS_GET_TICK        2   /* Get current tick count */
#define SYS_EXIT            3   /* Exit current task */

/* Semaphore operations */
#define SYS_SEM_WAIT        10  /* Wait on semaphore */
#define SYS_SEM_SIGNAL      11  /* Signal semaphore */
#define SYS_SEM_TRYWAIT     12  /* Try wait (non-blocking) */

/* Mutex operations */
#define SYS_MUTEX_LOCK      20  /* Lock mutex */
#define SYS_MUTEX_UNLOCK    21  /* Unlock mutex */
#define SYS_MUTEX_TRYLOCK   22  /* Try lock (non-blocking) */

/* Queue operations */
#define SYS_QUEUE_SEND      30  /* Send to queue */
#define SYS_QUEUE_RECV      31  /* Receive from queue */

/* I/O operations (via kernel) */
#define SYS_PUTCHAR         40  /* Output a character */
#define SYS_PUTS            41  /* Output a string */

/* Debug/info */
#define SYS_GET_TASK_ID     50  /* Get current task ID */

/* -----------------------------------------------------------------------------
 * User-mode API
 *
 * User-mode syscall wrappers are defined in user_syscall.h and implemented
 * in src/user/user_syscall.c (linked to USER region for U-mode access).
 *
 * Include <user_syscall.h> in user task code.
 * ---------------------------------------------------------------------------*/

/* -----------------------------------------------------------------------------
 * Kernel-side syscall handler (called from trap.c)
 * ---------------------------------------------------------------------------*/

/**
 * Handle a system call from U-mode
 * @param ctx Pointer to saved context (for reading args and setting return)
 * @return New stack pointer (may be different if task switch occurred)
 */
reg_t *syscall_handler(reg_t *ctx);

#endif /* DANIERTOS_SYSCALL_H */

