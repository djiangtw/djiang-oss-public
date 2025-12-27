/**
 *
 * SPDX-License-Identifier: MIT
 * Copyright (c) 2025 Danny Jiang
 * https://github.com/djiangtw/djiang-oss-public

 * interval.h - Interval Utilities
 *
 * Utilities for interval problems:
 *   - Sorting intervals (by start, by end)
 *   - Overlap detection
 *   - Merge intervals
 *   - Binary search on intervals
 *
 * Author: Danny Jiang
 * Created: 2025-12-24
 */

#ifndef DJLC_INTERVAL_H
#define DJLC_INTERVAL_H

#include <stddef.h>
#include <stdbool.h>
#include <stdlib.h>

/*============================================================================
 * INTERVAL STRUCTURE
 *===========================================================================*/

typedef struct {
    int start;
    int end;
    int idx;  /* Original index (useful for tracking) */
} djlc_interval_t;

/*============================================================================
 * COMPARISON FUNCTIONS for qsort
 *===========================================================================*/

/* Sort by start time ascending */
static inline int djlc_interval_cmp_by_start(const void *a, const void *b) {
    const djlc_interval_t *ia = (const djlc_interval_t *)a;
    const djlc_interval_t *ib = (const djlc_interval_t *)b;
    if (ia->start != ib->start) return ia->start - ib->start;
    return ia->end - ib->end;
}

/* Sort by end time ascending (greedy interval scheduling) */
static inline int djlc_interval_cmp_by_end(const void *a, const void *b) {
    const djlc_interval_t *ia = (const djlc_interval_t *)a;
    const djlc_interval_t *ib = (const djlc_interval_t *)b;
    if (ia->end != ib->end) return ia->end - ib->end;
    return ia->start - ib->start;
}

/* Sort int** intervals by start (for LeetCode format) */
static inline int djlc_interval_ptr_cmp_start(const void *a, const void *b) {
    return (*(int **)a)[0] - (*(int **)b)[0];
}

/* Sort int** intervals by end (for LeetCode format) */
static inline int djlc_interval_ptr_cmp_end(const void *a, const void *b) {
    return (*(int **)a)[1] - (*(int **)b)[1];
}

/*============================================================================
 * OVERLAP DETECTION
 *===========================================================================*/

/* Check if two intervals overlap */
static inline bool djlc_interval_overlaps(djlc_interval_t *a, djlc_interval_t *b) {
    return a->start < b->end && b->start < a->end;
}

/* Check if interval a contains interval b */
static inline bool djlc_interval_contains(djlc_interval_t *a, djlc_interval_t *b) {
    return a->start <= b->start && b->end <= a->end;
}

/* Merge two overlapping intervals into result */
static inline void djlc_interval_merge_two(djlc_interval_t *a, djlc_interval_t *b, 
                                           djlc_interval_t *result) {
    result->start = a->start < b->start ? a->start : b->start;
    result->end = a->end > b->end ? a->end : b->end;
    result->idx = a->idx;
}

/*============================================================================
 * MERGE INTERVALS
 * Input: sorted intervals by start
 * Returns: number of merged intervals
 *===========================================================================*/

static inline int djlc_interval_merge(djlc_interval_t *intervals, int n, 
                                      djlc_interval_t *result) {
    if (n == 0) return 0;
    result[0] = intervals[0];
    int count = 1;
    for (int i = 1; i < n; i++) {
        if (intervals[i].start <= result[count - 1].end) {
            /* Overlapping, merge */
            if (intervals[i].end > result[count - 1].end) {
                result[count - 1].end = intervals[i].end;
            }
        } else {
            /* No overlap, add new interval */
            result[count++] = intervals[i];
        }
    }
    return count;
}

/*============================================================================
 * NON-OVERLAPPING COUNT (Greedy)
 * Returns: minimum intervals to remove to make non-overlapping
 *===========================================================================*/

static inline int djlc_interval_min_remove(djlc_interval_t *intervals, int n) {
    if (n == 0) return 0;
    /* Sort by end time for greedy approach */
    qsort(intervals, n, sizeof(djlc_interval_t), djlc_interval_cmp_by_end);
    int end = intervals[0].end;
    int remove_count = 0;
    for (int i = 1; i < n; i++) {
        if (intervals[i].start < end) {
            remove_count++;
        } else {
            end = intervals[i].end;
        }
    }
    return remove_count;
}

/*============================================================================
 * BINARY SEARCH ON INTERVALS
 *===========================================================================*/

/* Find first interval with start >= target (lower bound) */
static inline int djlc_interval_lower_bound_start(djlc_interval_t *intervals, int n, 
                                                   int target) {
    int lo = 0, hi = n;
    while (lo < hi) {
        int mid = lo + (hi - lo) / 2;
        if (intervals[mid].start >= target) hi = mid;
        else lo = mid + 1;
    }
    return lo;
}

/* Find first interval with end > target (upper bound) */
static inline int djlc_interval_upper_bound_end(djlc_interval_t *intervals, int n,
                                                 int target) {
    int lo = 0, hi = n;
    while (lo < hi) {
        int mid = lo + (hi - lo) / 2;
        if (intervals[mid].end > target) hi = mid;
        else lo = mid + 1;
    }
    return lo;
}

#endif /* DJLC_INTERVAL_H */

