/**
 *
 * SPDX-License-Identifier: MIT
 * Copyright (c) 2025 Danny Jiang
 * https://github.com/djiangtw/djiang-oss-public

 * circular_buffer.h - Circular Buffer (Ring Buffer)
 *
 * Fixed-size buffer that wraps around. Useful for:
 *   - Moving average / sliding window sum
 *   - Bounded queues
 *   - Data streaming
 *
 * Author: Danny Jiang
 * Created: 2025-12-24
 */

#ifndef DJLC_CIRCULAR_BUFFER_H
#define DJLC_CIRCULAR_BUFFER_H

#include <stddef.h>
#include <stdbool.h>
#include <stdlib.h>

/*============================================================================
 * CIRCULAR BUFFER (INT)
 *===========================================================================*/

typedef struct {
    int *data;
    int capacity;   /* Fixed size of buffer */
    int count;      /* Current number of elements (max = capacity) */
    int head;       /* Index of oldest element */
    int tail;       /* Index where next element will be written */
} djlc_cbuf_int_t;

static inline bool djlc_cbuf_int_init(djlc_cbuf_int_t *cb, int capacity) {
    cb->data = (int *)calloc(capacity, sizeof(int));
    if (!cb->data) return false;
    cb->capacity = capacity;
    cb->count = 0;
    cb->head = 0;
    cb->tail = 0;
    return true;
}

static inline void djlc_cbuf_int_free(djlc_cbuf_int_t *cb) {
    if (cb->data) { free(cb->data); cb->data = NULL; }
    cb->capacity = cb->count = cb->head = cb->tail = 0;
}

static inline void djlc_cbuf_int_clear(djlc_cbuf_int_t *cb) {
    cb->count = cb->head = cb->tail = 0;
}

static inline bool djlc_cbuf_int_empty(djlc_cbuf_int_t *cb) {
    return cb->count == 0;
}

static inline bool djlc_cbuf_int_full(djlc_cbuf_int_t *cb) {
    return cb->count == cb->capacity;
}

static inline int djlc_cbuf_int_size(djlc_cbuf_int_t *cb) {
    return cb->count;
}

/* Push value, overwriting oldest if full. Returns overwritten value or 0 */
static inline int djlc_cbuf_int_push(djlc_cbuf_int_t *cb, int val) {
    int old_val = 0;
    if (cb->count == cb->capacity) {
        old_val = cb->data[cb->head];
        cb->head = (cb->head + 1) % cb->capacity;
    } else {
        cb->count++;
    }
    cb->data[cb->tail] = val;
    cb->tail = (cb->tail + 1) % cb->capacity;
    return old_val;
}

/* Pop oldest value */
static inline bool djlc_cbuf_int_pop(djlc_cbuf_int_t *cb, int *val) {
    if (cb->count == 0) return false;
    *val = cb->data[cb->head];
    cb->head = (cb->head + 1) % cb->capacity;
    cb->count--;
    return true;
}

/* Peek oldest value */
static inline bool djlc_cbuf_int_peek(djlc_cbuf_int_t *cb, int *val) {
    if (cb->count == 0) return false;
    *val = cb->data[cb->head];
    return true;
}

/* Get element at index (0 = oldest) */
static inline int djlc_cbuf_int_get(djlc_cbuf_int_t *cb, int idx) {
    return cb->data[(cb->head + idx) % cb->capacity];
}

/*============================================================================
 * MOVING AVERAGE using Circular Buffer
 *===========================================================================*/

typedef struct {
    djlc_cbuf_int_t buf;
    long long sum;
} djlc_moving_avg_t;

static inline bool djlc_moving_avg_init(djlc_moving_avg_t *ma, int window_size) {
    ma->sum = 0;
    return djlc_cbuf_int_init(&ma->buf, window_size);
}

static inline void djlc_moving_avg_free(djlc_moving_avg_t *ma) {
    djlc_cbuf_int_free(&ma->buf);
    ma->sum = 0;
}

static inline void djlc_moving_avg_clear(djlc_moving_avg_t *ma) {
    djlc_cbuf_int_clear(&ma->buf);
    ma->sum = 0;
}

/* Add value and return current average */
static inline double djlc_moving_avg_next(djlc_moving_avg_t *ma, int val) {
    int old_val = djlc_cbuf_int_push(&ma->buf, val);
    ma->sum += val - old_val;
    return (double)ma->sum / ma->buf.count;
}

/* Get current average without adding */
static inline double djlc_moving_avg_get(djlc_moving_avg_t *ma) {
    if (ma->buf.count == 0) return 0.0;
    return (double)ma->sum / ma->buf.count;
}

/* Get current sum */
static inline long long djlc_moving_avg_sum(djlc_moving_avg_t *ma) {
    return ma->sum;
}

/* Get current count */
static inline int djlc_moving_avg_count(djlc_moving_avg_t *ma) {
    return ma->buf.count;
}

#endif /* DJLC_CIRCULAR_BUFFER_H */

