/**
 * danieRTOS - Task Management API
 *
 * Copyright (c) 2025 Danny Jiang <djiang.tw@gmail.com>
 * SPDX-License-Identifier: MIT
 */

#ifndef DANIERTOS_TASK_H
#define DANIERTOS_TASK_H

#include "types.h"

/* -----------------------------------------------------------------------------
 * Task Control Block (TCB)
 * ---------------------------------------------------------------------------*/

struct tcb {
    /* Stack pointer (must be first for context switch) */
    reg_t       *sp;

    /* Task identity */
    uint8_t     id;
    const char  *name;

    /* Scheduling */
    priority_t  priority;
    priority_t  base_priority;      /* For priority inheritance */
    task_state_t state;

    /* Time management */
    tick_t      wake_tick;          /* Tick when task should wake */
    wake_reason_t wake_reason;

    /* Blocking */
    void        *blocked_on;        /* Semaphore/mutex/queue blocking this task */

    /* Linked list for ready queue and wait queues */
    tcb_t       *next;

    /* Stack info (for debugging/overflow detection) */
    reg_t       *stack_base;
    size_t      stack_size;
};

/* -----------------------------------------------------------------------------
 * Task API
 * ---------------------------------------------------------------------------*/

/**
 * Create a new task
 *
 * @param name      Task name (for debugging)
 * @param func      Task function
 * @param arg       Argument to pass to task function
 * @param priority  Task priority (0 = lowest)
 * @param stack     Stack memory
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

