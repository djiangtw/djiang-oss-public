/**
 * danieRTOS - Message Queue API
 *
 * Copyright (c) 2025 Danny Jiang <djiang.tw@gmail.com>
 * SPDX-License-Identifier: MIT
 */

#ifndef DANIERTOS_QUEUE_H
#define DANIERTOS_QUEUE_H

#include "types.h"

/* -----------------------------------------------------------------------------
 * Queue Structure
 * ---------------------------------------------------------------------------*/

struct queue {
    uint8_t     *buffer;        /* Message buffer */
    size_t      item_size;      /* Size of each item */
    size_t      capacity;       /* Maximum number of items */
    size_t      count;          /* Current number of items */
    size_t      head;           /* Read position */
    size_t      tail;           /* Write position */
    tcb_t       *send_wait;     /* Tasks waiting to send */
    tcb_t       *recv_wait;     /* Tasks waiting to receive */
};

/* -----------------------------------------------------------------------------
 * Queue API
 * ---------------------------------------------------------------------------*/

/**
 * Initialize a queue
 *
 * @param q         Pointer to queue structure
 * @param buffer    Buffer to store messages
 * @param item_size Size of each item in bytes
 * @param capacity  Maximum number of items
 */
void queue_init(queue_t *q, void *buffer, size_t item_size, size_t capacity);

/**
 * Send item to queue
 *
 * @param q             Pointer to queue
 * @param item          Pointer to item to send
 * @param timeout_ticks Maximum ticks to wait if full
 * @return              true if sent, false if timeout
 */
bool queue_send(queue_t *q, const void *item, uint32_t timeout_ticks);

/**
 * Send item to front of queue (high priority)
 *
 * @param q             Pointer to queue
 * @param item          Pointer to item to send
 * @param timeout_ticks Maximum ticks to wait if full
 * @return              true if sent, false if timeout
 */
bool queue_send_front(queue_t *q, const void *item, uint32_t timeout_ticks);

/**
 * Receive item from queue
 *
 * @param q             Pointer to queue
 * @param item          Pointer to buffer for received item
 * @param timeout_ticks Maximum ticks to wait if empty
 * @return              true if received, false if timeout
 */
bool queue_receive(queue_t *q, void *item, uint32_t timeout_ticks);

/**
 * Peek at front item without removing
 *
 * @param q     Pointer to queue
 * @param item  Pointer to buffer for item
 * @return      true if item available, false if empty
 */
bool queue_peek(queue_t *q, void *item);

/**
 * Get number of items in queue
 */
size_t queue_count(queue_t *q);

/**
 * Get available space in queue
 */
size_t queue_space(queue_t *q);

/**
 * Check if queue is empty
 */
bool queue_is_empty(queue_t *q);

/**
 * Check if queue is full
 */
bool queue_is_full(queue_t *q);

/**
 * Reset queue (clear all items)
 */
void queue_reset(queue_t *q);

#endif /* DANIERTOS_QUEUE_H */

