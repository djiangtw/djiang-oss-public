/**
 * danieRTOS - Mutex API
 *
 * Copyright (c) 2025 Danny Jiang <djiang.tw@gmail.com>
 * SPDX-License-Identifier: MIT
 *
 * Mutexes provide mutual exclusion with priority inheritance
 * to prevent priority inversion.
 */

#ifndef DANIERTOS_MUTEX_H
#define DANIERTOS_MUTEX_H

#include "types.h"

/* -----------------------------------------------------------------------------
 * Mutex Structure
 * ---------------------------------------------------------------------------*/

struct mutex {
    tcb_t       *owner;         /* Task that owns the mutex (NULL if unlocked) */
    tcb_t       *wait_queue;    /* Tasks waiting on this mutex */
    uint8_t     lock_count;     /* For recursive locking */
};

/* -----------------------------------------------------------------------------
 * Mutex API
 * ---------------------------------------------------------------------------*/

/**
 * Initialize a mutex
 *
 * @param mtx   Pointer to mutex structure
 */
void mutex_init(mutex_t *mtx);

/**
 * Lock a mutex
 *
 * If the mutex is owned by another task, blocks until it becomes available.
 * Implements priority inheritance: if a higher priority task blocks on a
 * mutex held by a lower priority task, the owner's priority is raised.
 *
 * @param mtx           Pointer to mutex
 * @param timeout_ticks Maximum ticks to wait (WAIT_FOREVER for infinite)
 * @return              true if locked, false if timeout
 */
bool mutex_lock(mutex_t *mtx, uint32_t timeout_ticks);

/**
 * Try to lock mutex without blocking
 *
 * @param mtx   Pointer to mutex
 * @return      true if locked, false if would block
 */
bool mutex_try_lock(mutex_t *mtx);

/**
 * Unlock a mutex
 *
 * Must be called by the owner. Restores owner's priority if it was
 * elevated by priority inheritance.
 *
 * @param mtx   Pointer to mutex
 */
void mutex_unlock(mutex_t *mtx);

/**
 * Check if mutex is locked
 *
 * @param mtx   Pointer to mutex
 * @return      true if locked
 */
bool mutex_is_locked(mutex_t *mtx);

/**
 * Get mutex owner
 *
 * @param mtx   Pointer to mutex
 * @return      Owner task, or NULL if unlocked
 */
tcb_t *mutex_get_owner(mutex_t *mtx);

#endif /* DANIERTOS_MUTEX_H */

