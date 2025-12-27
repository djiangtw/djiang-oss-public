/**
 *
 * SPDX-License-Identifier: MIT
 * Copyright (c) 2025 Danny Jiang
 * https://github.com/djiangtw/djiang-oss-public

 * random.h - Random Utilities
 *
 * Random number generation and shuffling utilities.
 * Includes Fisher-Yates shuffle, random sampling, etc.
 *
 * Author: Danny Jiang
 * Created: 2025-12-24
 */

#ifndef DJLC_RANDOM_H
#define DJLC_RANDOM_H

#include <stddef.h>
#include <stdbool.h>
#include <stdlib.h>
#include <time.h>

/*============================================================================
 * RANDOM NUMBER GENERATION
 *===========================================================================*/

/* Initialize random seed (call once at program start) */
static inline void djlc_random_seed(unsigned int seed) {
    srand(seed);
}

/* Initialize with current time */
static inline void djlc_random_seed_time(void) {
    srand((unsigned int)time(NULL));
}

/* Random integer in range [0, n-1] */
static inline int djlc_random_int(int n) {
    return rand() % n;
}

/* Random integer in range [min, max] inclusive */
static inline int djlc_random_range(int min, int max) {
    return min + rand() % (max - min + 1);
}

/* Random double in range [0.0, 1.0) */
static inline double djlc_random_double(void) {
    return (double)rand() / ((double)RAND_MAX + 1.0);
}

/* Random double in range [min, max) */
static inline double djlc_random_double_range(double min, double max) {
    return min + djlc_random_double() * (max - min);
}

/*============================================================================
 * FISHER-YATES SHUFFLE (in-place)
 * Time: O(n), Space: O(1)
 *===========================================================================*/

/* Shuffle int array in-place */
static inline void djlc_shuffle_int(int *arr, int n) {
    for (int i = n - 1; i > 0; i--) {
        int j = rand() % (i + 1);
        int temp = arr[i];
        arr[i] = arr[j];
        arr[j] = temp;
    }
}

/* Shuffle pointer array in-place */
static inline void djlc_shuffle_ptr(void **arr, int n) {
    for (int i = n - 1; i > 0; i--) {
        int j = rand() % (i + 1);
        void *temp = arr[i];
        arr[i] = arr[j];
        arr[j] = temp;
    }
}

/* Shuffle char array in-place (for strings) */
static inline void djlc_shuffle_char(char *arr, int n) {
    for (int i = n - 1; i > 0; i--) {
        int j = rand() % (i + 1);
        char temp = arr[i];
        arr[i] = arr[j];
        arr[j] = temp;
    }
}

/*============================================================================
 * RESERVOIR SAMPLING
 * Select k random elements from stream of unknown size
 *===========================================================================*/

typedef struct {
    int *reservoir;
    int k;
    int count;
} djlc_reservoir_t;

static inline bool djlc_reservoir_init(djlc_reservoir_t *rs, int k) {
    rs->reservoir = (int *)malloc(k * sizeof(int));
    if (!rs->reservoir) return false;
    rs->k = k;
    rs->count = 0;
    return true;
}

static inline void djlc_reservoir_free(djlc_reservoir_t *rs) {
    if (rs->reservoir) { free(rs->reservoir); rs->reservoir = NULL; }
    rs->k = rs->count = 0;
}

static inline void djlc_reservoir_reset(djlc_reservoir_t *rs) {
    rs->count = 0;
}

/* Add element to reservoir */
static inline void djlc_reservoir_add(djlc_reservoir_t *rs, int val) {
    if (rs->count < rs->k) {
        rs->reservoir[rs->count] = val;
    } else {
        int j = rand() % (rs->count + 1);
        if (j < rs->k) {
            rs->reservoir[j] = val;
        }
    }
    rs->count++;
}

/* Get random element from reservoir (after stream ends) */
static inline int djlc_reservoir_get(djlc_reservoir_t *rs, int idx) {
    return rs->reservoir[idx];
}

/*============================================================================
 * RANDOM PICK WITH WEIGHT
 * Time: O(log n) per pick after O(n) preprocessing
 *===========================================================================*/

typedef struct {
    int *prefix_sum;
    int total;
    int n;
} djlc_weighted_random_t;

static inline bool djlc_weighted_random_init(djlc_weighted_random_t *wr, int *weights, int n) {
    wr->prefix_sum = (int *)malloc(n * sizeof(int));
    if (!wr->prefix_sum) return false;
    wr->n = n;
    wr->total = 0;
    for (int i = 0; i < n; i++) {
        wr->total += weights[i];
        wr->prefix_sum[i] = wr->total;
    }
    return true;
}

static inline void djlc_weighted_random_free(djlc_weighted_random_t *wr) {
    if (wr->prefix_sum) { free(wr->prefix_sum); wr->prefix_sum = NULL; }
    wr->n = wr->total = 0;
}

/* Pick random index based on weights (binary search) */
static inline int djlc_weighted_random_pick(djlc_weighted_random_t *wr) {
    int target = rand() % wr->total + 1;
    int lo = 0, hi = wr->n - 1;
    while (lo < hi) {
        int mid = lo + (hi - lo) / 2;
        if (wr->prefix_sum[mid] < target) lo = mid + 1;
        else hi = mid;
    }
    return lo;
}

#endif /* DJLC_RANDOM_H */

