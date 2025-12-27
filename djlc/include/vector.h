/**
 *
 * SPDX-License-Identifier: MIT
 * Copyright (c) 2025 Danny Jiang
 * https://github.com/djiangtw/djiang-oss-public

 * vector.h - Dynamic Array (Vector)
 *
 * A generic dynamic array that grows automatically.
 * Stores void* pointers, can be used for any data type.
 *
 * Usage:
 *   djlc_vector_t vec;
 *   djlc_vec_init(&vec, 16);
 *   djlc_vec_push(&vec, ptr);
 *   void* item = djlc_vec_get(&vec, index);
 *   djlc_vec_free(&vec);
 *
 * Author: Danny Jiang
 * Created: 2025-12-17
 */

#ifndef DJLC_VECTOR_H
#define DJLC_VECTOR_H

#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#define DJLC_VEC_DEFAULT_CAP 16

typedef struct {
    void** data;
    int size;
    int cap;
} djlc_vector_t;

/* Initialize vector with given capacity */
static inline bool djlc_vec_init(djlc_vector_t* vec, int cap) {
    vec->data = (void**)malloc(cap * sizeof(void*));
    if (!vec->data) return false;
    vec->size = 0;
    vec->cap = cap;
    return true;
}

/* Initialize with default capacity */
static inline bool djlc_vec_init_default(djlc_vector_t* vec) {
    return djlc_vec_init(vec, DJLC_VEC_DEFAULT_CAP);
}

/* Free vector (does NOT free elements) */
static inline void djlc_vec_free(djlc_vector_t* vec) {
    free(vec->data);
    vec->data = NULL;
    vec->size = 0;
    vec->cap = 0;
}

/* Ensure capacity for at least n more elements */
static inline bool djlc_vec_reserve(djlc_vector_t* vec, int n) {
    if (vec->size + n <= vec->cap) return true;
    int new_cap = vec->cap;
    while (new_cap < vec->size + n) new_cap *= 2;
    void** new_data = (void**)realloc(vec->data, new_cap * sizeof(void*));
    if (!new_data) return false;
    vec->data = new_data;
    vec->cap = new_cap;
    return true;
}

/* Push element to end */
static inline bool djlc_vec_push(djlc_vector_t* vec, void* elem) {
    if (!djlc_vec_reserve(vec, 1)) return false;
    vec->data[vec->size++] = elem;
    return true;
}

/* Pop element from end */
static inline void* djlc_vec_pop(djlc_vector_t* vec) {
    if (vec->size == 0) return NULL;
    return vec->data[--vec->size];
}

/* Get element at index */
static inline void* djlc_vec_get(djlc_vector_t* vec, int index) {
    if (index < 0 || index >= vec->size) return NULL;
    return vec->data[index];
}

/* Set element at index */
static inline bool djlc_vec_set(djlc_vector_t* vec, int index, void* elem) {
    if (index < 0 || index >= vec->size) return false;
    vec->data[index] = elem;
    return true;
}

/* Get size */
static inline int djlc_vec_size(djlc_vector_t* vec) {
    return vec->size;
}

/* Check if empty */
static inline bool djlc_vec_empty(djlc_vector_t* vec) {
    return vec->size == 0;
}

/* Clear (reset size, keep capacity) */
static inline void djlc_vec_clear(djlc_vector_t* vec) {
    vec->size = 0;
}

/* ========== Integer Vector (convenience) ========== */

typedef struct {
    int* data;
    int size;
    int cap;
} djlc_vec_int_t;

static inline bool djlc_vec_int_init(djlc_vec_int_t* vec, int cap) {
    vec->data = (int*)malloc(cap * sizeof(int));
    if (!vec->data) return false;
    vec->size = 0;
    vec->cap = cap;
    return true;
}

static inline void djlc_vec_int_free(djlc_vec_int_t* vec) {
    free(vec->data);
    vec->data = NULL;
    vec->size = 0;
    vec->cap = 0;
}

static inline bool djlc_vec_int_push(djlc_vec_int_t* vec, int val) {
    if (vec->size == vec->cap) {
        int new_cap = vec->cap * 2;
        int* new_data = (int*)realloc(vec->data, new_cap * sizeof(int));
        if (!new_data) return false;
        vec->data = new_data;
        vec->cap = new_cap;
    }
    vec->data[vec->size++] = val;
    return true;
}

static inline int djlc_vec_int_pop(djlc_vec_int_t* vec) {
    return vec->data[--vec->size];
}

static inline void djlc_vec_int_clear(djlc_vec_int_t* vec) {
    vec->size = 0;
}

#endif /* DJLC_VECTOR_H */

