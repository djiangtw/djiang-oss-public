/**
 *
 * SPDX-License-Identifier: MIT
 * Copyright (c) 2025 Danny Jiang
 * https://github.com/djiangtw/djiang-oss-public

 * matrix.h - Matrix Utility Functions
 *
 * Common matrix operations for LeetCode problems.
 * Includes: transpose, rotate, flip, etc.
 *
 * Note: All functions assume square matrix (n x n) unless specified.
 *
 * Author: Danny Jiang
 * Created: 2025-12-17
 */

#ifndef DJLC_MATRIX_H
#define DJLC_MATRIX_H

#include <stdlib.h>
#include <string.h>

/* ========== In-place Operations (Square Matrix) ========== */

/* Transpose matrix in-place (swap m[i][j] with m[j][i]) */
static inline void djlc_matrix_transpose(int** m, int n) {
    for (int i = 0; i < n; i++) {
        for (int j = i + 1; j < n; j++) {
            int t = m[i][j];
            m[i][j] = m[j][i];
            m[j][i] = t;
        }
    }
}

/* Reverse each row (horizontal flip) */
static inline void djlc_matrix_reverse_rows(int** m, int n) {
    for (int i = 0; i < n; i++) {
        for (int j = 0, k = n - 1; j < k; j++, k--) {
            int t = m[i][j];
            m[i][j] = m[i][k];
            m[i][k] = t;
        }
    }
}

/* Reverse each column (vertical flip) */
static inline void djlc_matrix_reverse_cols(int** m, int n) {
    for (int j = 0; j < n; j++) {
        for (int i = 0, k = n - 1; i < k; i++, k--) {
            int t = m[i][j];
            m[i][j] = m[k][j];
            m[k][j] = t;
        }
    }
}

/* Rotate 90 degrees clockwise: transpose + reverse rows */
static inline void djlc_matrix_rotate_90_cw(int** m, int n) {
    djlc_matrix_transpose(m, n);
    djlc_matrix_reverse_rows(m, n);
}

/* Rotate 90 degrees counter-clockwise: transpose + reverse columns */
static inline void djlc_matrix_rotate_90_ccw(int** m, int n) {
    djlc_matrix_transpose(m, n);
    djlc_matrix_reverse_cols(m, n);
}

/* Rotate 180 degrees: reverse rows + reverse columns */
static inline void djlc_matrix_rotate_180(int** m, int n) {
    djlc_matrix_reverse_rows(m, n);
    djlc_matrix_reverse_cols(m, n);
}

/* ========== Layer-based Rotation (alternative method) ========== */

/* Rotate 90 degrees clockwise using 4-way swap (in-place) */
static inline void djlc_matrix_rotate_90_layer(int** m, int n) {
    for (int layer = 0; layer < n / 2; layer++) {
        int first = layer;
        int last = n - 1 - layer;
        for (int i = first; i < last; i++) {
            int offset = i - first;
            /* save top */
            int top = m[first][i];
            /* left -> top */
            m[first][i] = m[last - offset][first];
            /* bottom -> left */
            m[last - offset][first] = m[last][last - offset];
            /* right -> bottom */
            m[last][last - offset] = m[i][last];
            /* top -> right */
            m[i][last] = top;
        }
    }
}

/* ========== Matrix Creation/Free ========== */

/* Allocate n x m matrix */
static inline int** djlc_matrix_alloc(int rows, int cols) {
    int** m = (int**)malloc(rows * sizeof(int*));
    if (!m) return NULL;
    for (int i = 0; i < rows; i++) {
        m[i] = (int*)calloc(cols, sizeof(int));
        if (!m[i]) {
            for (int j = 0; j < i; j++) free(m[j]);
            free(m);
            return NULL;
        }
    }
    return m;
}

/* Free n x m matrix */
static inline void djlc_matrix_free(int** m, int rows) {
    if (!m) return;
    for (int i = 0; i < rows; i++) {
        free(m[i]);
    }
    free(m);
}

/* Copy matrix */
static inline int** djlc_matrix_copy(int** src, int rows, int cols) {
    int** dst = djlc_matrix_alloc(rows, cols);
    if (!dst) return NULL;
    for (int i = 0; i < rows; i++) {
        memcpy(dst[i], src[i], cols * sizeof(int));
    }
    return dst;
}

/* ========== Print (for debugging) ========== */

#include <stdio.h>

static inline void djlc_matrix_print(int** m, int rows, int cols) {
    for (int i = 0; i < rows; i++) {
        for (int j = 0; j < cols; j++) {
            printf("%3d ", m[i][j]);
        }
        printf("\n");
    }
}

#endif /* DJLC_MATRIX_H */

