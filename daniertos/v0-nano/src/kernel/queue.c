/**
 * danieRTOS - Message Queue Implementation
 *
 * Copyright (c) 2025 Danny Jiang <djiang.tw@gmail.com>
 * SPDX-License-Identifier: MIT
 */

#include "daniertos.h"

/* -----------------------------------------------------------------------------
 * Wait Queue Helpers
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
    if (*queue == NULL) return NULL;
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
 * Queue API Implementation
 * ---------------------------------------------------------------------------*/

void queue_init(queue_t *q, void *buffer, size_t item_size, size_t capacity)
{
    q->buffer = (uint8_t *)buffer;
    q->item_size = item_size;
    q->capacity = capacity;
    q->count = 0;
    q->head = 0;
    q->tail = 0;
    q->send_wait = NULL;
    q->recv_wait = NULL;
}

bool queue_send(queue_t *q, const void *item, uint32_t timeout_ticks)
{
    reg_t state = critical_enter();

    /* Try to send */
    if (q->count < q->capacity) {
        memcpy(&q->buffer[q->tail * q->item_size], item, q->item_size);
        q->tail = (q->tail + 1) % q->capacity;
        q->count++;

        /* Wake receiver if any */
        tcb_t *task = waitqueue_pop(&q->recv_wait);
        if (task != NULL) {
            task->wake_reason = WAKE_REASON_SIGNALED;
            task_make_ready(task);
            sched_request_switch();
        }

        critical_exit(state);
        return true;
    }

    /* Queue full */
    if (timeout_ticks == NO_WAIT) {
        critical_exit(state);
        return false;
    }

    tcb_t *current = task_get_current();
    waitqueue_add(&q->send_wait, current);
    task_block(current, q);

    if (timeout_ticks != WAIT_FOREVER) {
        delay_add(current, tick_get() + timeout_ticks);
    }

    critical_exit(state);
    task_yield();

    /* Check result */
    state = critical_enter();
    bool sent = (current->wake_reason == WAKE_REASON_SIGNALED);
    if (!sent) {
        waitqueue_remove(&q->send_wait, current);
    }
    critical_exit(state);

    /* If woken by signal, actually send now */
    if (sent) {
        return queue_send(q, item, NO_WAIT);
    }
    return false;
}

bool queue_send_front(queue_t *q, const void *item, uint32_t timeout_ticks)
{
    reg_t state = critical_enter();

    if (q->count < q->capacity) {
        /* Insert at front */
        q->head = (q->head == 0) ? q->capacity - 1 : q->head - 1;
        memcpy(&q->buffer[q->head * q->item_size], item, q->item_size);
        q->count++;

        tcb_t *task = waitqueue_pop(&q->recv_wait);
        if (task != NULL) {
            task->wake_reason = WAKE_REASON_SIGNALED;
            task_make_ready(task);
            sched_request_switch();
        }

        critical_exit(state);
        return true;
    }

    critical_exit(state);

    /* Fall back to regular send with timeout */
    if (timeout_ticks == NO_WAIT) {
        return false;
    }

    /* Block and retry (simplified - same as queue_send) */
    return queue_send(q, item, timeout_ticks);
}

bool queue_receive(queue_t *q, void *item, uint32_t timeout_ticks)
{
    reg_t state = critical_enter();

    if (q->count > 0) {
        memcpy(item, &q->buffer[q->head * q->item_size], q->item_size);
        q->head = (q->head + 1) % q->capacity;
        q->count--;

        tcb_t *task = waitqueue_pop(&q->send_wait);
        if (task != NULL) {
            task->wake_reason = WAKE_REASON_SIGNALED;
            task_make_ready(task);
            sched_request_switch();
        }

        critical_exit(state);
        return true;
    }

    if (timeout_ticks == NO_WAIT) {
        critical_exit(state);
        return false;
    }

    tcb_t *current = task_get_current();
    waitqueue_add(&q->recv_wait, current);
    task_block(current, q);

    if (timeout_ticks != WAIT_FOREVER) {
        delay_add(current, tick_get() + timeout_ticks);
    }

    critical_exit(state);
    task_yield();

    state = critical_enter();
    bool received = (current->wake_reason == WAKE_REASON_SIGNALED);
    if (!received) {
        waitqueue_remove(&q->recv_wait, current);
    }
    critical_exit(state);

    if (received) {
        return queue_receive(q, item, NO_WAIT);
    }
    return false;
}

bool queue_peek(queue_t *q, void *item)
{
    reg_t state = critical_enter();

    if (q->count == 0) {
        critical_exit(state);
        return false;
    }

    memcpy(item, &q->buffer[q->head * q->item_size], q->item_size);
    critical_exit(state);
    return true;
}

size_t queue_count(queue_t *q)
{
    return q->count;
}

size_t queue_space(queue_t *q)
{
    return q->capacity - q->count;
}

bool queue_is_empty(queue_t *q)
{
    return q->count == 0;
}

bool queue_is_full(queue_t *q)
{
    return q->count >= q->capacity;
}

void queue_reset(queue_t *q)
{
    reg_t state = critical_enter();

    q->count = 0;
    q->head = 0;
    q->tail = 0;

    /* Wake all waiting senders */
    while (q->send_wait != NULL) {
        tcb_t *task = waitqueue_pop(&q->send_wait);
        task->wake_reason = WAKE_REASON_TIMEOUT;  /* Not exactly right but signals failure */
        task_make_ready(task);
    }

    /* Wake all waiting receivers */
    while (q->recv_wait != NULL) {
        tcb_t *task = waitqueue_pop(&q->recv_wait);
        task->wake_reason = WAKE_REASON_TIMEOUT;
        task_make_ready(task);
    }

    critical_exit(state);
}
