/**
 * danieRTOS - Scheduler API
 *
 * Copyright (c) 2025 Danny Jiang <djiang.tw@gmail.com>
 * SPDX-License-Identifier: MIT
 */

#ifndef DANIERTOS_SCHEDULER_H
#define DANIERTOS_SCHEDULER_H

#include "types.h"
#include "task.h"

/* -----------------------------------------------------------------------------
 * Scheduler State
 * ---------------------------------------------------------------------------*/

typedef enum {
    SCHED_STATE_STOPPED = 0,
    SCHED_STATE_RUNNING = 1
} sched_state_t;

/* -----------------------------------------------------------------------------
 * Scheduler API
 * ---------------------------------------------------------------------------*/

/**
 * Initialize the scheduler
 */
void sched_init(void);

/**
 * Start the scheduler (does not return)
 */
void sched_start(void);

/**
 * Get scheduler state
 */
sched_state_t sched_get_state(void);

/**
 * Check if we're in an ISR context
 */
bool sched_in_isr(void);

/* -----------------------------------------------------------------------------
 * Internal (used by kernel)
 * ---------------------------------------------------------------------------*/

/**
 * Add task to ready queue
 */
void sched_add_ready(tcb_t *task);

/**
 * Remove task from ready queue
 */
void sched_remove_ready(tcb_t *task);

/**
 * Add task to delayed list (for task_delay)
 */
void sched_add_delayed(tcb_t *task);

/**
 * Select next task to run and perform context switch
 * Called from timer interrupt
 *
 * @param current_sp  Current stack pointer
 * @return           New stack pointer (may be same if no switch)
 */
reg_t *sched_schedule(reg_t *current_sp);

/**
 * Request a context switch at next opportunity
 */
void sched_request_switch(void);

/**
 * Perform yield (immediate context switch)
 * @param current_sp  Current stack pointer
 * @return           New stack pointer
 */
reg_t *sched_yield(reg_t *current_sp);

#endif /* DANIERTOS_SCHEDULER_H */

