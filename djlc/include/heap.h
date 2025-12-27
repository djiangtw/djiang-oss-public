/**
 *
 * SPDX-License-Identifier: MIT
 * Copyright (c) 2025 Danny Jiang
 * https://github.com/djiangtw/djiang-oss-public

 * heap.h - Binary Heap / Priority Queue
 *
 * Generic min-heap and max-heap implementations.
 * Supports push, pop, peek, and heapify operations.
 *
 * Author: Danny Jiang
 * Created: 2025-12-24
 */

#ifndef DJLC_HEAP_H
#define DJLC_HEAP_H

#include <stddef.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

/*============================================================================
 * INT HEAP - Min or Max heap for integers
 *===========================================================================*/

typedef struct {
    int *data;
    int size;
    int capacity;
    bool is_max;  /* true = max-heap, false = min-heap */
} djlc_heap_int_t;

static inline bool djlc_heap_int_init(djlc_heap_int_t *h, int capacity, bool is_max) {
    h->data = (int *)malloc(capacity * sizeof(int));
    if (!h->data) return false;
    h->size = 0;
    h->capacity = capacity;
    h->is_max = is_max;
    return true;
}

static inline void djlc_heap_int_free(djlc_heap_int_t *h) {
    if (h->data) { free(h->data); h->data = NULL; }
    h->size = 0;
    h->capacity = 0;
}

static inline bool djlc_heap_int_empty(djlc_heap_int_t *h) {
    return h->size == 0;
}

static inline int djlc_heap_int_size(djlc_heap_int_t *h) {
    return h->size;
}

/* Compare: returns true if a should be above b in heap */
static inline bool djlc_heap_int_cmp(djlc_heap_int_t *h, int a, int b) {
    return h->is_max ? (a > b) : (a < b);
}

static inline bool djlc_heap_int_push(djlc_heap_int_t *h, int val) {
    if (h->size >= h->capacity) {
        int new_cap = h->capacity * 2;
        int *new_data = (int *)realloc(h->data, new_cap * sizeof(int));
        if (!new_data) return false;
        h->data = new_data;
        h->capacity = new_cap;
    }
    /* Bubble up */
    int i = h->size++;
    while (i > 0) {
        int p = (i - 1) / 2;
        if (!djlc_heap_int_cmp(h, val, h->data[p])) break;
        h->data[i] = h->data[p];
        i = p;
    }
    h->data[i] = val;
    return true;
}

static inline bool djlc_heap_int_pop(djlc_heap_int_t *h, int *val) {
    if (h->size == 0) return false;
    *val = h->data[0];
    int last = h->data[--h->size];
    if (h->size == 0) return true;
    
    /* Bubble down */
    int i = 0;
    while (2 * i + 1 < h->size) {
        int c = 2 * i + 1;
        if (c + 1 < h->size && djlc_heap_int_cmp(h, h->data[c + 1], h->data[c])) c++;
        if (!djlc_heap_int_cmp(h, h->data[c], last)) break;
        h->data[i] = h->data[c];
        i = c;
    }
    h->data[i] = last;
    return true;
}

static inline bool djlc_heap_int_peek(djlc_heap_int_t *h, int *val) {
    if (h->size == 0) return false;
    *val = h->data[0];
    return true;
}

/* Heapify an existing array in-place */
static inline void djlc_heap_int_heapify(djlc_heap_int_t *h) {
    for (int i = h->size / 2 - 1; i >= 0; i--) {
        int val = h->data[i];
        int idx = i;
        while (2 * idx + 1 < h->size) {
            int c = 2 * idx + 1;
            if (c + 1 < h->size && djlc_heap_int_cmp(h, h->data[c + 1], h->data[c])) c++;
            if (!djlc_heap_int_cmp(h, h->data[c], val)) break;
            h->data[idx] = h->data[c];
            idx = c;
        }
        h->data[idx] = val;
    }
}

/*============================================================================
 * Convenience macros for common heap types
 *===========================================================================*/

#define djlc_min_heap_int_init(h, cap) djlc_heap_int_init(h, cap, false)
#define djlc_max_heap_int_init(h, cap) djlc_heap_int_init(h, cap, true)

#endif /* DJLC_HEAP_H */

