/**
 *
 * SPDX-License-Identifier: MIT
 * Copyright (c) 2025 Danny Jiang
 * https://github.com/djiangtw/djiang-oss-public

 * stack.h - Stack data structure
 *
 * Array-based stack implementations for common types.
 * Provides both static (fixed-size) and dynamic versions.
 *
 * == Common Algorithm Patterns ==
 *
 * 1. Monotonic Stack - Subarray Contribution (LC 0907, 0084, 0739):
 *    For each element, calculate how many subarrays have it as min/max.
 *    - left = distance to previous smaller/larger
 *    - right = distance to next smaller/larger
 *    - contribution = arr[i] * left * right
 *
 * 2. Next Greater/Smaller Element (LC 0496, 0503):
 *    while (top > 0 && arr[stack[top-1]] < arr[i]) {
 *        result[stack[--top]] = arr[i];  // arr[i] is next greater
 *    }
 *    stack[top++] = i;
 *
 * Author: Danny Jiang
 * Created: 2025-12-15
 */

#ifndef DJLC_STACK_H
#define DJLC_STACK_H

#include <stddef.h>
#include <stdbool.h>
#include <stdlib.h>

/*============================================================================
 * STATIC STACK (char) - Fixed size, no malloc
 * Usage: For problems like Valid Parentheses where max size is known
 *===========================================================================*/

#define DJLC_STACK_CHAR_STATIC_DEFINE(name, capacity) \
    char name##_data[capacity];                       \
    int name##_top = -1

#define DJLC_STACK_CHAR_PUSH(name, val) \
    (name##_data[++name##_top] = (val))

#define DJLC_STACK_CHAR_POP(name) \
    (name##_data[name##_top--])

#define DJLC_STACK_CHAR_PEEK(name) \
    (name##_data[name##_top])

#define DJLC_STACK_CHAR_EMPTY(name) \
    (name##_top == -1)

#define DJLC_STACK_CHAR_SIZE(name) \
    (name##_top + 1)

/*============================================================================
 * DYNAMIC STACK (int) - Growable
 *===========================================================================*/

typedef struct {
    int *data;
    int top;
    int capacity;
} djlc_stack_int_t;

static inline bool djlc_stack_int_init(djlc_stack_int_t *s, int capacity) {
    s->data = (int *)malloc(capacity * sizeof(int));
    if (!s->data) return false;
    s->top = -1;
    s->capacity = capacity;
    return true;
}

static inline void djlc_stack_int_free(djlc_stack_int_t *s) {
    if (s->data) {
        free(s->data);
        s->data = NULL;
    }
    s->top = -1;
    s->capacity = 0;
}

static inline bool djlc_stack_int_push(djlc_stack_int_t *s, int val) {
    if (s->top + 1 >= s->capacity) {
        /* Grow by 2x */
        int new_cap = s->capacity * 2;
        int *new_data = (int *)realloc(s->data, new_cap * sizeof(int));
        if (!new_data) return false;
        s->data = new_data;
        s->capacity = new_cap;
    }
    s->data[++s->top] = val;
    return true;
}

static inline bool djlc_stack_int_pop(djlc_stack_int_t *s, int *val) {
    if (s->top == -1) return false;
    *val = s->data[s->top--];
    return true;
}

static inline bool djlc_stack_int_peek(djlc_stack_int_t *s, int *val) {
    if (s->top == -1) return false;
    *val = s->data[s->top];
    return true;
}

static inline bool djlc_stack_int_empty(djlc_stack_int_t *s) {
    return s->top == -1;
}

static inline int djlc_stack_int_size(djlc_stack_int_t *s) {
    return s->top + 1;
}

/*============================================================================
 * DYNAMIC STACK (char) - Growable
 *===========================================================================*/

typedef struct {
    char *data;
    int top;
    int capacity;
} djlc_stack_char_t;

static inline bool djlc_stack_char_init(djlc_stack_char_t *s, int capacity) {
    s->data = (char *)malloc(capacity * sizeof(char));
    if (!s->data) return false;
    s->top = -1;
    s->capacity = capacity;
    return true;
}

static inline void djlc_stack_char_free(djlc_stack_char_t *s) {
    if (s->data) {
        free(s->data);
        s->data = NULL;
    }
    s->top = -1;
    s->capacity = 0;
}

static inline bool djlc_stack_char_push(djlc_stack_char_t *s, char val) {
    if (s->top + 1 >= s->capacity) {
        int new_cap = s->capacity * 2;
        char *new_data = (char *)realloc(s->data, new_cap * sizeof(char));
        if (!new_data) return false;
        s->data = new_data;
        s->capacity = new_cap;
    }
    s->data[++s->top] = val;
    return true;
}

static inline bool djlc_stack_char_pop(djlc_stack_char_t *s, char *val) {
    if (s->top == -1) return false;
    *val = s->data[s->top--];
    return true;
}

static inline bool djlc_stack_char_peek(djlc_stack_char_t *s, char *val) {
    if (s->top == -1) return false;
    *val = s->data[s->top];
    return true;
}

static inline bool djlc_stack_char_empty(djlc_stack_char_t *s) {
    return s->top == -1;
}

static inline int djlc_stack_char_size(djlc_stack_char_t *s) {
    return s->top + 1;
}

#endif /* DJLC_STACK_H */

