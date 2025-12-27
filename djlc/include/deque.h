/**
 *
 * SPDX-License-Identifier: MIT
 * Copyright (c) 2025 Danny Jiang
 * https://github.com/djiangtw/djiang-oss-public

 * deque.h - Double-ended Queue (Deque)
 *
 * Array-based deque supporting push/pop from both ends.
 * Ideal for monotonic queue and sliding window problems.
 *
 * == Common Algorithm Patterns ==
 *
 * 1. Sliding Window Maximum (LC 0239):
 *    - Maintain decreasing order of values (store indices)
 *    - Pop front if outside window: if (front <= i - k) pop_front()
 *    - Pop back while <= current: while (nums[back] <= nums[i]) pop_back()
 *    - Answer is nums[front]
 *
 * 2. Shortest Subarray with Sum >= K (LC 0862):
 *    - Use prefix sums with monotonic increasing deque
 *    - Pop front while prefix[i] - prefix[front] >= k (valid answer)
 *    - Pop back while prefix[back] >= prefix[i] (no need for larger prefix)
 *
 * Author: Danny Jiang
 * Created: 2025-12-24
 */

#ifndef DJLC_DEQUE_H
#define DJLC_DEQUE_H

#include <stddef.h>
#include <stdbool.h>
#include <stdlib.h>

/*============================================================================
 * INT DEQUE - For sliding window, monotonic queue
 *===========================================================================*/

typedef struct {
    int *data;
    int front;    /* Index of first element */
    int rear;     /* Index after last element */
    int size;
    int capacity;
} djlc_deque_int_t;

static inline bool djlc_deque_int_init(djlc_deque_int_t *dq, int capacity) {
    dq->data = (int *)malloc(capacity * sizeof(int));
    if (!dq->data) return false;
    dq->front = 0;
    dq->rear = 0;
    dq->size = 0;
    dq->capacity = capacity;
    return true;
}

static inline void djlc_deque_int_free(djlc_deque_int_t *dq) {
    if (dq->data) { free(dq->data); dq->data = NULL; }
    dq->front = dq->rear = dq->size = dq->capacity = 0;
}

static inline void djlc_deque_int_clear(djlc_deque_int_t *dq) {
    dq->front = dq->rear = dq->size = 0;
}

static inline bool djlc_deque_int_empty(djlc_deque_int_t *dq) {
    return dq->size == 0;
}

static inline int djlc_deque_int_size(djlc_deque_int_t *dq) {
    return dq->size;
}

/* Push to back */
static inline bool djlc_deque_int_push_back(djlc_deque_int_t *dq, int val) {
    if (dq->size >= dq->capacity) {
        int new_cap = dq->capacity * 2;
        int *new_data = (int *)malloc(new_cap * sizeof(int));
        if (!new_data) return false;
        for (int i = 0; i < dq->size; i++) {
            new_data[i] = dq->data[(dq->front + i) % dq->capacity];
        }
        free(dq->data);
        dq->data = new_data;
        dq->front = 0;
        dq->rear = dq->size;
        dq->capacity = new_cap;
    }
    dq->data[dq->rear] = val;
    dq->rear = (dq->rear + 1) % dq->capacity;
    dq->size++;
    return true;
}

/* Push to front */
static inline bool djlc_deque_int_push_front(djlc_deque_int_t *dq, int val) {
    if (dq->size >= dq->capacity) {
        int new_cap = dq->capacity * 2;
        int *new_data = (int *)malloc(new_cap * sizeof(int));
        if (!new_data) return false;
        for (int i = 0; i < dq->size; i++) {
            new_data[i + 1] = dq->data[(dq->front + i) % dq->capacity];
        }
        free(dq->data);
        dq->data = new_data;
        dq->front = 1;
        dq->rear = dq->size + 1;
        dq->capacity = new_cap;
    }
    dq->front = (dq->front - 1 + dq->capacity) % dq->capacity;
    dq->data[dq->front] = val;
    dq->size++;
    return true;
}

/* Pop from back */
static inline bool djlc_deque_int_pop_back(djlc_deque_int_t *dq, int *val) {
    if (dq->size == 0) return false;
    dq->rear = (dq->rear - 1 + dq->capacity) % dq->capacity;
    *val = dq->data[dq->rear];
    dq->size--;
    return true;
}

/* Pop from front */
static inline bool djlc_deque_int_pop_front(djlc_deque_int_t *dq, int *val) {
    if (dq->size == 0) return false;
    *val = dq->data[dq->front];
    dq->front = (dq->front + 1) % dq->capacity;
    dq->size--;
    return true;
}

/* Peek front */
static inline bool djlc_deque_int_front(djlc_deque_int_t *dq, int *val) {
    if (dq->size == 0) return false;
    *val = dq->data[dq->front];
    return true;
}

/* Peek back */
static inline bool djlc_deque_int_back(djlc_deque_int_t *dq, int *val) {
    if (dq->size == 0) return false;
    *val = dq->data[(dq->rear - 1 + dq->capacity) % dq->capacity];
    return true;
}

/*============================================================================
 * MONOTONIC DEQUE HELPERS - For sliding window maximum/minimum
 *===========================================================================*/

/* Pop back while back >= val (for monotonic increasing - find min) */
static inline void djlc_deque_int_pop_back_while_ge(djlc_deque_int_t *dq, int val) {
    int back;
    while (dq->size > 0 && djlc_deque_int_back(dq, &back) && back >= val) {
        djlc_deque_int_pop_back(dq, &back);
    }
}

/* Pop back while back <= val (for monotonic decreasing - find max) */
static inline void djlc_deque_int_pop_back_while_le(djlc_deque_int_t *dq, int val) {
    int back;
    while (dq->size > 0 && djlc_deque_int_back(dq, &back) && back <= val) {
        djlc_deque_int_pop_back(dq, &back);
    }
}

#endif /* DJLC_DEQUE_H */

