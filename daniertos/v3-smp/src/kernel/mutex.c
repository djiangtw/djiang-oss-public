/**
 * danieRTOS - Mutex Implementation with Priority Inheritance
 *
 * Copyright (c) 2025 Danny Jiang <djiang.tw@gmail.com>
 * SPDX-License-Identifier: MIT
 */

#include "daniertos.h"

/* -----------------------------------------------------------------------------
 * Wait Queue Helpers (same as semaphore)
 * ---------------------------------------------------------------------------*/

static void waitqueue_add(tcb_t **queue, tcb_t *task)
{
    tcb_t **pp = queue;
    while (*pp != NULL && (*pp)->priority >= task->priority) {
        pp = &(*pp)->next;
    }
    task->next = *pp;
    *pp = task;
}

static tcb_t *waitqueue_pop(tcb_t **queue)
{
    if (*queue == NULL) {
        return NULL;
    }
    tcb_t *task = *queue;
    *queue = task->next;
    task->next = NULL;
    return task;
}

static void waitqueue_remove(tcb_t **queue, tcb_t *task)
{
    tcb_t **pp = queue;
    while (*pp != NULL) {
        if (*pp == task) {
            *pp = task->next;
            task->next = NULL;
            return;
        }
        pp = &(*pp)->next;
    }
}

/* -----------------------------------------------------------------------------
 * Mutex API Implementation
 * ---------------------------------------------------------------------------*/

void mutex_init(mutex_t *mtx)
{
    mtx->owner = NULL;
    mtx->wait_queue = NULL;
    mtx->lock_count = 0;
}

bool mutex_lock(mutex_t *mtx, uint32_t timeout_ticks)
{
    reg_t state = critical_enter();
    tcb_t *current = task_get_current();

    /* Check for recursive locking */
    if (mtx->owner == current) {
        mtx->lock_count++;
        critical_exit(state);
        return true;
    }

    /* Try to acquire */
    if (mtx->owner == NULL) {
        mtx->owner = current;
        mtx->lock_count = 1;
        critical_exit(state);
        return true;
    }

    /* Mutex is owned by another task */
    if (timeout_ticks == NO_WAIT) {
        critical_exit(state);
        return false;
    }

    /* Priority inheritance: boost owner's priority if needed */
    if (mtx->owner->priority < current->priority) {
        mtx->owner->priority = current->priority;
        /* Re-sort owner in ready queue */
        if (mtx->owner->state == TASK_STATE_READY) {
            sched_remove_ready(mtx->owner);
            sched_add_ready(mtx->owner);
        }
    }

    /* Block on mutex */
    waitqueue_add(&mtx->wait_queue, current);
    task_block(current, mtx);

    /* Set up timeout if specified */
    if (timeout_ticks != WAIT_FOREVER) {
        delay_add(current, tick_get() + timeout_ticks);
    }

    critical_exit(state);
    task_yield();

    /* Woke up - check result */
    state = critical_enter();

    bool acquired = (current->wake_reason == WAKE_REASON_SIGNALED);

    if (acquired) {
        /* Acquired - remove from delay list if we had a timeout */
        if (timeout_ticks != WAIT_FOREVER) {
            delay_remove(current);
        }
    } else {
        /* Timeout - remove from wait queue */
        waitqueue_remove(&mtx->wait_queue, current);
    }

    critical_exit(state);

    return acquired;
}

bool mutex_try_lock(mutex_t *mtx)
{
    return mutex_lock(mtx, NO_WAIT);
}

void mutex_unlock(mutex_t *mtx)
{
    reg_t state = critical_enter();
    tcb_t *current = task_get_current();

    /* Must be owner */
    if (mtx->owner != current) {
        critical_exit(state);
        return;
    }

    /* Handle recursive unlock */
    if (mtx->lock_count > 1) {
        mtx->lock_count--;
        critical_exit(state);
        return;
    }

    /* Restore original priority (undo priority inheritance) */
    current->priority = current->base_priority;

    /* Wake highest priority waiting task */
    tcb_t *next_owner = waitqueue_pop(&mtx->wait_queue);

    if (next_owner != NULL) {
        mtx->owner = next_owner;
        mtx->lock_count = 1;
        next_owner->wake_reason = WAKE_REASON_SIGNALED;
        task_make_ready(next_owner);
        sched_request_switch();
    } else {
        mtx->owner = NULL;
        mtx->lock_count = 0;
    }

    critical_exit(state);
}

bool mutex_is_locked(mutex_t *mtx)
{
    return mtx->owner != NULL;
}

tcb_t *mutex_get_owner(mutex_t *mtx)
{
    return mtx->owner;
}
