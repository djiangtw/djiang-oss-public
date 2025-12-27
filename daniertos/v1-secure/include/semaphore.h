/**
 * danieRTOS - Semaphore API
 *
 * Copyright (c) 2025 Danny Jiang <djiang.tw@gmail.com>
 * SPDX-License-Identifier: MIT
 */

#ifndef DANIERTOS_SEMAPHORE_H
#define DANIERTOS_SEMAPHORE_H

#include "types.h"

/* -----------------------------------------------------------------------------
 * Semaphore Structure
 * ---------------------------------------------------------------------------*/

struct semaphore {
    int32_t     count;          /* Current count (>0: available, <=0: blocked) */
    int32_t     max_count;      /* Maximum count (for counting semaphores) */
    tcb_t       *wait_queue;    /* Tasks waiting on this semaphore */
};

/* -----------------------------------------------------------------------------
 * Semaphore API
 * ---------------------------------------------------------------------------*/

/**
 * Initialize a binary semaphore
 *
 * @param sem           Pointer to semaphore structure
 * @param initial_count Initial count (0 or 1 for binary)
 */
void sem_init(semaphore_t *sem, int32_t initial_count);

/**
 * Initialize a counting semaphore
 *
 * @param sem           Pointer to semaphore structure
 * @param max_count     Maximum count
 * @param initial_count Initial count
 */
void sem_init_counting(semaphore_t *sem, int32_t max_count, int32_t initial_count);

/**
 * Wait on semaphore (P operation / down / acquire)
 *
 * Decrements the semaphore count. If count becomes negative,
 * the calling task is blocked until the semaphore is signaled.
 *
 * @param sem           Pointer to semaphore
 * @param timeout_ticks Maximum ticks to wait (WAIT_FOREVER for infinite)
 * @return              true if acquired, false if timeout
 */
bool sem_wait(semaphore_t *sem, uint32_t timeout_ticks);

/**
 * Try to acquire semaphore without blocking
 *
 * @param sem   Pointer to semaphore
 * @return      true if acquired, false if would block
 */
bool sem_try_wait(semaphore_t *sem);

/**
 * Signal semaphore (V operation / up / release)
 *
 * Increments the semaphore count. If tasks are waiting,
 * wakes the highest priority waiting task.
 *
 * @param sem   Pointer to semaphore
 */
void sem_signal(semaphore_t *sem);

/**
 * Get current semaphore count
 *
 * @param sem   Pointer to semaphore
 * @return      Current count
 */
int32_t sem_get_count(semaphore_t *sem);

#endif /* DANIERTOS_SEMAPHORE_H */

