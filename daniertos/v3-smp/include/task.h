/**
 * danieRTOS v3.x - Task Management API
 *
 * Copyright (c) 2025 Danny Jiang <djiang.tw@gmail.com>
 * SPDX-License-Identifier: MIT
 *
 * v3.x: Integrates SMP (v2.x) with User Mode (v1.x)
 *   - Per-Task Kernel Stack (for trap handling and kernel blocking)
 *   - Per-Task User Stack (for U-mode execution)
 *   - Stack info for dynamic PMP configuration
 */

#ifndef DANIERTOS_TASK_H
#define DANIERTOS_TASK_H

#include "types.h"

/* -----------------------------------------------------------------------------
 * Task Control Block (TCB) - v3.x
 *
 * Key differences from v2:
 *   - kstack_base/kstack_top: Per-Task Kernel Stack (used by trap entry)
 *   - ustack_base/ustack_size: Per-Task User Stack (used for PMP config)
 *   - held_mutexes: Resource ownership tracking for fault isolation
 * ---------------------------------------------------------------------------*/

/* Forward declaration for mutex ownership tracking */
struct mutex;

struct tcb {
    /* Stack pointer (must be first for context switch) */
    reg_t       *sp;

    /* Task identity */
    uint8_t     id;
    const char  *name;

    /* --- Stack Pointers (v3.x: Per-Task Dual Stack) --- */
    reg_t       *kstack_base;       /* Kernel Stack base address */
    reg_t       *kstack_top;        /* Kernel Stack top (used by trap entry) */
    reg_t       *ustack_base;       /* User Stack base address */
    size_t      ustack_size;        /* User Stack size (for PMP) */

    /* Scheduling */
    priority_t  priority;
    priority_t  base_priority;      /* For priority inheritance */
    task_state_t state;

    /* SMP: Core affinity */
    uint32_t    affinity_mask;      /* Bitmask of allowed cores */

    /* Time management */
    tick_t      wake_tick;          /* Tick when task should wake */
    wake_reason_t wake_reason;

    /* Blocking */
    void        *blocked_on;        /* Semaphore/mutex/queue blocking this task */

    /* --- Resource Ownership Tracking (v3.x: Fault Isolation) --- */
    struct mutex *held_mutexes;     /* Linked list of held mutexes */

    /* Linked list pointers (separate to avoid conflicts) */
    tcb_t       *next;              /* For ready queue and wait queues */
    tcb_t       *delay_next;        /* For delay list only */

    /* Legacy stack info (kept for v2 compatibility) */
    reg_t       *stack_base;        /* Same as ustack_base for v3 */
    size_t      stack_size;         /* Same as ustack_size for v3 */
};

/* -----------------------------------------------------------------------------
 * Task API
 * ---------------------------------------------------------------------------*/

/* Task privilege levels (v3.x) */
#define TASK_PRIV_M     0   /* M-mode (kernel) task */
#define TASK_PRIV_U     1   /* U-mode (user) task */

/**
 * Create a new task (v2 compatibility API - M-mode task)
 *
 * @param name      Task name (for debugging)
 * @param func      Task function
 * @param arg       Argument to pass to task function
 * @param priority  Task priority (0 = lowest)
 * @param stack     Stack memory
 * @param stack_size Stack size in bytes
 * @param affinity  Core affinity mask (CONFIG_CORE_ANY for any core)
 * @return          Pointer to TCB, or NULL on failure
 */
tcb_t *task_create(const char *name,
                   task_func_t func,
                   void *arg,
                   priority_t priority,
                   void *stack,
                   size_t stack_size,
                   uint32_t affinity);

#if CONFIG_USER_MODE
/**
 * Create a new user-mode task (v3.x)
 *
 * @param name       Task name
 * @param func       Task entry point (in user region)
 * @param arg        Task argument
 * @param priority   Task priority
 * @param affinity   Core affinity mask
 * @return           Pointer to TCB, or NULL on failure
 *
 * Note: Kernel stack and user stack are automatically allocated.
 */
tcb_t *task_create_user(const char *name,
                        task_func_t func,
                        void *arg,
                        priority_t priority,
                        uint32_t affinity);
#endif

/**
 * Delete a task
 */
void task_delete(tcb_t *task);

/**
 * Get current running task
 */
tcb_t *task_get_current(void);

/**
 * Yield CPU to another ready task of same or higher priority
 */
void task_yield(void);

/**
 * Delay task for specified number of ticks
 */
void task_delay(tick_t ticks);

/**
 * Internal delay setup (no yield) - used by syscall handler
 */
void task_delay_internal(tick_t ticks);

/**
 * Exit current task (called via syscall for U-mode tasks)
 */
void task_exit(void);

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

