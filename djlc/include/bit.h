/**
 *
 * SPDX-License-Identifier: MIT
 * Copyright (c) 2025 Danny Jiang
 * https://github.com/djiangtw/djiang-oss-public

 * bit.h - Binary Indexed Tree (Fenwick Tree)
 *
 * Efficient data structure for:
 *   - Point update + Range sum query: O(log n)
 *   - Range update + Point query: O(log n)
 *   - Inversion count, 2D range queries
 *
 * Author: Danny Jiang
 * Created: 2025-12-24
 */

#ifndef DJLC_BIT_H
#define DJLC_BIT_H

#include <stddef.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

/*============================================================================
 * BINARY INDEXED TREE (1D)
 * Note: Internally uses 1-based indexing
 *===========================================================================*/

typedef struct {
    int *tree;
    int n;        /* Size of original array */
} djlc_bit_t;

/* Get lowest set bit */
static inline int djlc_bit_lowbit(int x) {
    return x & (-x);
}

static inline bool djlc_bit_init(djlc_bit_t *bit, int n) {
    bit->n = n;
    bit->tree = (int *)calloc(n + 1, sizeof(int));
    return bit->tree != NULL;
}

static inline void djlc_bit_free(djlc_bit_t *bit) {
    if (bit->tree) { free(bit->tree); bit->tree = NULL; }
    bit->n = 0;
}

static inline void djlc_bit_clear(djlc_bit_t *bit) {
    memset(bit->tree, 0, (bit->n + 1) * sizeof(int));
}

/* Build BIT from array (0-indexed input) */
static inline void djlc_bit_build(djlc_bit_t *bit, int *arr, int n) {
    djlc_bit_clear(bit);
    for (int i = 0; i < n; i++) {
        int idx = i + 1;
        int delta = arr[i];
        while (idx <= bit->n) {
            bit->tree[idx] += delta;
            idx += djlc_bit_lowbit(idx);
        }
    }
}

/* Point update: arr[i] += delta (0-indexed) */
static inline void djlc_bit_update(djlc_bit_t *bit, int i, int delta) {
    int idx = i + 1;
    while (idx <= bit->n) {
        bit->tree[idx] += delta;
        idx += djlc_bit_lowbit(idx);
    }
}

/* Prefix sum: sum(arr[0..i]) (0-indexed, inclusive) */
static inline int djlc_bit_query(djlc_bit_t *bit, int i) {
    int sum = 0;
    int idx = i + 1;
    while (idx > 0) {
        sum += bit->tree[idx];
        idx -= djlc_bit_lowbit(idx);
    }
    return sum;
}

/* Range sum: sum(arr[left..right]) (0-indexed, inclusive) */
static inline int djlc_bit_range_sum(djlc_bit_t *bit, int left, int right) {
    if (left == 0) return djlc_bit_query(bit, right);
    return djlc_bit_query(bit, right) - djlc_bit_query(bit, left - 1);
}

/* Set value at index (requires knowing old value or computing it) */
static inline void djlc_bit_set(djlc_bit_t *bit, int i, int val) {
    int old_val = djlc_bit_range_sum(bit, i, i);
    djlc_bit_update(bit, i, val - old_val);
}

/*============================================================================
 * BINARY INDEXED TREE (2D)
 * For 2D range sum queries
 *===========================================================================*/

typedef struct {
    int **tree;
    int rows;
    int cols;
} djlc_bit_2d_t;

static inline bool djlc_bit_2d_init(djlc_bit_2d_t *bit, int rows, int cols) {
    bit->rows = rows;
    bit->cols = cols;
    bit->tree = (int **)malloc((rows + 1) * sizeof(int *));
    if (!bit->tree) return false;
    for (int i = 0; i <= rows; i++) {
        bit->tree[i] = (int *)calloc(cols + 1, sizeof(int));
        if (!bit->tree[i]) {
            for (int j = 0; j < i; j++) free(bit->tree[j]);
            free(bit->tree);
            return false;
        }
    }
    return true;
}

static inline void djlc_bit_2d_free(djlc_bit_2d_t *bit) {
    for (int i = 0; i <= bit->rows; i++) free(bit->tree[i]);
    free(bit->tree);
    bit->tree = NULL;
    bit->rows = bit->cols = 0;
}

/* Point update: matrix[row][col] += delta (0-indexed) */
static inline void djlc_bit_2d_update(djlc_bit_2d_t *bit, int row, int col, int delta) {
    for (int i = row + 1; i <= bit->rows; i += djlc_bit_lowbit(i)) {
        for (int j = col + 1; j <= bit->cols; j += djlc_bit_lowbit(j)) {
            bit->tree[i][j] += delta;
        }
    }
}

/* Prefix sum: sum(matrix[0..row][0..col]) (0-indexed, inclusive) */
static inline int djlc_bit_2d_query(djlc_bit_2d_t *bit, int row, int col) {
    int sum = 0;
    for (int i = row + 1; i > 0; i -= djlc_bit_lowbit(i)) {
        for (int j = col + 1; j > 0; j -= djlc_bit_lowbit(j)) {
            sum += bit->tree[i][j];
        }
    }
    return sum;
}

/* Range sum: sum(matrix[r1..r2][c1..c2]) (0-indexed, inclusive) */
static inline int djlc_bit_2d_range_sum(djlc_bit_2d_t *bit, int r1, int c1, int r2, int c2) {
    int sum = djlc_bit_2d_query(bit, r2, c2);
    if (r1 > 0) sum -= djlc_bit_2d_query(bit, r1 - 1, c2);
    if (c1 > 0) sum -= djlc_bit_2d_query(bit, r2, c1 - 1);
    if (r1 > 0 && c1 > 0) sum += djlc_bit_2d_query(bit, r1 - 1, c1 - 1);
    return sum;
}

#endif /* DJLC_BIT_H */

