/**
 * danieRTOS v1.x - Task Management API
 *
 * Copyright (c) 2025 Danny Jiang <djiang.tw@gmail.com>
 * SPDX-License-Identifier: MIT
 *
 * v1.x: Dual stack support for M+U mode
 */

#ifndef DANIERTOS_TASK_H
#define DANIERTOS_TASK_H

#include "types.h"
#include "config.h"

/* -----------------------------------------------------------------------------
 * Task Control Block (TCB)
 *
 * v1.x: Added kernel_sp for dual stack architecture
 *   - sp: User stack pointer (for U-mode execution)
 *   - kernel_sp: Kernel stack pointer (for M-mode trap handling)
 *
 * Stack layout per task:
 *   +------------------+
 *   | User Stack       | <- sp (task runs here in U-mode)
 *   +------------------+
 *   | Kernel Stack     | <- kernel_sp (trap handler uses this)
 *   +------------------+
 * ---------------------------------------------------------------------------*/

struct tcb {
    /* Stack pointers (must be first for context switch) */
    reg_t       *sp;                /* User stack pointer */
#if CONFIG_USER_MODE
    reg_t       *kernel_sp;         /* Kernel stack pointer (M-mode) */
#endif

    /* Task identity */
    uint8_t     id;
    const char  *name;

    /* Scheduling */
    priority_t  priority;
    priority_t  base_priority;      /* For priority inheritance */
    task_state_t state;

#if CONFIG_USER_MODE
    /* Privilege mode (for mixed M/U mode tasks) */
    uint8_t     privilege;          /* 0 = U-mode, 3 = M-mode */
#endif

    /* Time management */
    tick_t      wake_tick;          /* Tick when task should wake */
    wake_reason_t wake_reason;

    /* Blocking */
    void        *blocked_on;        /* Semaphore/mutex/queue blocking this task */

    /* Linked list for ready queue and wait queues */
    tcb_t       *next;

    /* Stack info (for debugging/overflow detection) */
    reg_t       *stack_base;        /* User stack base */
    size_t      stack_size;         /* User stack size */
#if CONFIG_USER_MODE
    reg_t       *kernel_stack_base; /* Kernel stack base */
    size_t      kernel_stack_size;  /* Kernel stack size */
#endif
};

/* Privilege mode constants */
#define TASK_PRIV_U     0           /* User mode */
#define TASK_PRIV_M     3           /* Machine mode */

/* -----------------------------------------------------------------------------
 * Task API
 * ---------------------------------------------------------------------------*/

/**
 * Create a new task (runs in U-mode by default in v1.x)
 *
 * @param name      Task name (for debugging)
 * @param func      Task function
 * @param arg       Argument to pass to task function
 * @param priority  Task priority (0 = lowest)
 * @param stack     Stack memory (user stack)
 * @param stack_size Stack size in bytes
 * @return          Pointer to TCB, or NULL on failure
 */
tcb_t *task_create(const char *name,
                   task_func_t func,
                   void *arg,
                   priority_t priority,
                   void *stack,
                   size_t stack_size);

/**
 * Create a task with specified privilege level (v1.x)
 *
 * @param name      Task name
 * @param func      Task function
 * @param arg       Argument to pass to task function
 * @param priority  Task priority
 * @param stack     Stack memory (user stack)
 * @param stack_size Stack size in bytes
 * @param privilege TASK_PRIV_U or TASK_PRIV_M
 * @return          Pointer to TCB, or NULL on failure
 */
tcb_t *task_create_priv(const char *name,
                        task_func_t func,
                        void *arg,
                        priority_t priority,
                        void *stack,
                        size_t stack_size,
                        uint8_t privilege);

/**
 * Delete a task
 */
void task_delete(tcb_t *task);

/**
 * Exit current task (called from syscall or task return)
 */
void task_exit(void);

/**
 * Delay current task for specified ticks
 */
void task_delay(tick_t ticks);

/**
 * Get current running task
 */
tcb_t *task_get_current(void);

/**
 * Yield CPU to another ready task of same or higher priority
 */
void task_yield(void);

/**
 * Suspend a task
 */
void task_suspend(tcb_t *task);

/**
 * Resume a suspended task
 */
void task_resume(tcb_t *task);

/**
 * Get task state
 */
task_state_t task_get_state(tcb_t *task);

/**
 * Set task priority
 */
void task_set_priority(tcb_t *task, priority_t priority);

/**
 * Get task priority
 */
priority_t task_get_priority(tcb_t *task);

/* -----------------------------------------------------------------------------
 * Internal (used by kernel)
 * ---------------------------------------------------------------------------*/

void task_init(void);
void task_make_ready(tcb_t *task);
void task_block(tcb_t *task, void *blocked_on);

#endif /* DANIERTOS_TASK_H */

