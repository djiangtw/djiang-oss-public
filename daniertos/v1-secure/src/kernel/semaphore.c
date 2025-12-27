/**
 * danieRTOS - Semaphore Implementation
 *
 * Copyright (c) 2025 Danny Jiang <djiang.tw@gmail.com>
 * SPDX-License-Identifier: MIT
 */

#include "daniertos.h"

/* -----------------------------------------------------------------------------
 * Wait Queue Helpers
 * ---------------------------------------------------------------------------*/

/**
 * Add task to wait queue (priority ordered - highest first)
 */
static void waitqueue_add(tcb_t **queue, tcb_t *task)
{
    tcb_t **pp = queue;

    /* Insert by priority (highest first) */
    while (*pp != NULL && (*pp)->priority >= task->priority) {
        pp = &(*pp)->next;
    }

    task->next = *pp;
    *pp = task;
}

/**
 * Remove highest priority task from wait queue
 */
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

/**
 * Remove specific task from wait queue
 */
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
 * Semaphore API Implementation
 * ---------------------------------------------------------------------------*/

void sem_init(semaphore_t *sem, int32_t initial_count)
{
    sem->count = initial_count;
    sem->max_count = 1;  /* Binary semaphore */
    sem->wait_queue = NULL;
}

void sem_init_counting(semaphore_t *sem, int32_t max_count, int32_t initial_count)
{
    sem->count = initial_count;
    sem->max_count = max_count;
    sem->wait_queue = NULL;
}

bool sem_wait(semaphore_t *sem, uint32_t timeout_ticks)
{
    reg_t state = critical_enter();

    /* Try to acquire */
    if (sem->count > 0) {
        sem->count--;
        critical_exit(state);
        return true;
    }

    /* Need to block */
    if (timeout_ticks == NO_WAIT) {
        critical_exit(state);
        return false;
    }

    tcb_t *current = task_get_current();

    /* Add to wait queue */
    waitqueue_add(&sem->wait_queue, current);
    task_block(current, sem);

    /* Set up timeout if specified */
    if (timeout_ticks != WAIT_FOREVER) {
        delay_add(current, tick_get() + timeout_ticks);
    }

    critical_exit(state);

    /* Yield to let other tasks run */
    task_yield();

    /* We're back - check why we woke up */
    state = critical_enter();

    bool acquired = (current->wake_reason == WAKE_REASON_SIGNALED);

    if (!acquired && current->wake_reason == WAKE_REASON_TIMEOUT) {
        /* Timeout - remove from wait queue */
        waitqueue_remove(&sem->wait_queue, current);
    }

    critical_exit(state);

    return acquired;
}

bool sem_try_wait(semaphore_t *sem)
{
    return sem_wait(sem, NO_WAIT);
}

void sem_signal(semaphore_t *sem)
{
    reg_t state = critical_enter();

    /* Wake highest priority waiting task */
    tcb_t *task = waitqueue_pop(&sem->wait_queue);

    if (task != NULL) {
        task->wake_reason = WAKE_REASON_SIGNALED;
        task_make_ready(task);
        sched_request_switch();
    } else {
        /* No waiting task - increment count */
        if (sem->count < sem->max_count) {
            sem->count++;
        }
    }

    critical_exit(state);
}

int32_t sem_get_count(semaphore_t *sem)
{
    return sem->count;
}
