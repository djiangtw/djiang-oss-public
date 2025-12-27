/**
 *
 * SPDX-License-Identifier: MIT
 * Copyright (c) 2025 Danny Jiang
 * https://github.com/djiangtw/djiang-oss-public

 * queue.h - Queue data structure
 *
 * Array-based circular queue for BFS and level-order traversal.
 * Supports int and void* (for TreeNode, etc.)
 *
 * Author: Danny Jiang
 * Created: 2025-12-24
 */

#ifndef DJLC_QUEUE_H
#define DJLC_QUEUE_H

#include <stddef.h>
#include <stdbool.h>
#include <stdlib.h>

/*============================================================================
 * INT QUEUE - For BFS on graphs with integer node IDs
 *===========================================================================*/

typedef struct {
    int *data;
    int front;
    int rear;
    int size;
    int capacity;
} djlc_queue_int_t;

static inline bool djlc_queue_int_init(djlc_queue_int_t *q, int capacity) {
    q->data = (int *)malloc(capacity * sizeof(int));
    if (!q->data) return false;
    q->front = 0;
    q->rear = 0;
    q->size = 0;
    q->capacity = capacity;
    return true;
}

static inline void djlc_queue_int_free(djlc_queue_int_t *q) {
    if (q->data) { free(q->data); q->data = NULL; }
    q->front = q->rear = q->size = q->capacity = 0;
}

static inline bool djlc_queue_int_empty(djlc_queue_int_t *q) {
    return q->size == 0;
}

static inline int djlc_queue_int_size(djlc_queue_int_t *q) {
    return q->size;
}

static inline bool djlc_queue_int_push(djlc_queue_int_t *q, int val) {
    if (q->size >= q->capacity) {
        /* Grow by 2x */
        int new_cap = q->capacity * 2;
        int *new_data = (int *)malloc(new_cap * sizeof(int));
        if (!new_data) return false;
        /* Copy in order */
        for (int i = 0; i < q->size; i++) {
            new_data[i] = q->data[(q->front + i) % q->capacity];
        }
        free(q->data);
        q->data = new_data;
        q->front = 0;
        q->rear = q->size;
        q->capacity = new_cap;
    }
    q->data[q->rear] = val;
    q->rear = (q->rear + 1) % q->capacity;
    q->size++;
    return true;
}

static inline bool djlc_queue_int_pop(djlc_queue_int_t *q, int *val) {
    if (q->size == 0) return false;
    *val = q->data[q->front];
    q->front = (q->front + 1) % q->capacity;
    q->size--;
    return true;
}

static inline bool djlc_queue_int_front(djlc_queue_int_t *q, int *val) {
    if (q->size == 0) return false;
    *val = q->data[q->front];
    return true;
}

/*============================================================================
 * POINTER QUEUE - For BFS on trees (TreeNode*, etc.)
 *===========================================================================*/

typedef struct {
    void **data;
    int front;
    int rear;
    int size;
    int capacity;
} djlc_queue_ptr_t;

static inline bool djlc_queue_ptr_init(djlc_queue_ptr_t *q, int capacity) {
    q->data = (void **)malloc(capacity * sizeof(void *));
    if (!q->data) return false;
    q->front = 0;
    q->rear = 0;
    q->size = 0;
    q->capacity = capacity;
    return true;
}

static inline void djlc_queue_ptr_free(djlc_queue_ptr_t *q) {
    if (q->data) { free(q->data); q->data = NULL; }
    q->front = q->rear = q->size = q->capacity = 0;
}

static inline bool djlc_queue_ptr_empty(djlc_queue_ptr_t *q) {
    return q->size == 0;
}

static inline int djlc_queue_ptr_size(djlc_queue_ptr_t *q) {
    return q->size;
}

static inline bool djlc_queue_ptr_push(djlc_queue_ptr_t *q, void *val) {
    if (q->size >= q->capacity) {
        int new_cap = q->capacity * 2;
        void **new_data = (void **)malloc(new_cap * sizeof(void *));
        if (!new_data) return false;
        for (int i = 0; i < q->size; i++) {
            new_data[i] = q->data[(q->front + i) % q->capacity];
        }
        free(q->data);
        q->data = new_data;
        q->front = 0;
        q->rear = q->size;
        q->capacity = new_cap;
    }
    q->data[q->rear] = val;
    q->rear = (q->rear + 1) % q->capacity;
    q->size++;
    return true;
}

static inline bool djlc_queue_ptr_pop(djlc_queue_ptr_t *q, void **val) {
    if (q->size == 0) return false;
    *val = q->data[q->front];
    q->front = (q->front + 1) % q->capacity;
    q->size--;
    return true;
}

static inline bool djlc_queue_ptr_front(djlc_queue_ptr_t *q, void **val) {
    if (q->size == 0) return false;
    *val = q->data[q->front];
    return true;
}

#endif /* DJLC_QUEUE_H */

