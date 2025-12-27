/**
 *
 * SPDX-License-Identifier: MIT
 * Copyright (c) 2025 Danny Jiang
 * https://github.com/djiangtw/djiang-oss-public

 * compare.h - Comparison functions for qsort
 *
 * Common comparison functions used with qsort().
 * All functions follow the qsort comparator signature:
 *   int cmp(const void* a, const void* b)
 *
 * Return value:
 *   < 0 if a < b
 *   = 0 if a == b
 *   > 0 if a > b
 *
 * Author: Danny Jiang
 * Created: 2025-12-17
 */

#ifndef DJLC_COMPARE_H
#define DJLC_COMPARE_H

#include <string.h>

/* ========== Integer Comparators ========== */

/* Ascending order (default) */
static inline int djlc_cmp_int_asc(const void* a, const void* b) {
    int ia = *(const int*)a;
    int ib = *(const int*)b;
    return (ia > ib) - (ia < ib);  /* avoid overflow */
}

/* Descending order */
static inline int djlc_cmp_int_desc(const void* a, const void* b) {
    int ia = *(const int*)a;
    int ib = *(const int*)b;
    return (ib > ia) - (ib < ia);
}

/* By absolute value (ascending) */
static inline int djlc_cmp_int_abs(const void* a, const void* b) {
    int ia = *(const int*)a;
    int ib = *(const int*)b;
    ia = ia < 0 ? -ia : ia;
    ib = ib < 0 ? -ib : ib;
    return (ia > ib) - (ia < ib);
}

/* ========== Long Comparators ========== */

static inline int djlc_cmp_long_asc(const void* a, const void* b) {
    long la = *(const long*)a;
    long lb = *(const long*)b;
    return (la > lb) - (la < lb);
}

static inline int djlc_cmp_long_desc(const void* a, const void* b) {
    long la = *(const long*)a;
    long lb = *(const long*)b;
    return (lb > la) - (lb < la);
}

/* ========== Character Comparators ========== */

static inline int djlc_cmp_char_asc(const void* a, const void* b) {
    return *(const char*)a - *(const char*)b;
}

static inline int djlc_cmp_char_desc(const void* a, const void* b) {
    return *(const char*)b - *(const char*)a;
}

/* ========== String Comparators ========== */

/* Lexicographic order */
static inline int djlc_cmp_str(const void* a, const void* b) {
    return strcmp(*(const char**)a, *(const char**)b);
}

/* By string length */
static inline int djlc_cmp_str_len(const void* a, const void* b) {
    size_t la = strlen(*(const char**)a);
    size_t lb = strlen(*(const char**)b);
    return (la > lb) - (la < lb);
}

/* ========== Double Comparators ========== */

static inline int djlc_cmp_double_asc(const void* a, const void* b) {
    double da = *(const double*)a;
    double db = *(const double*)b;
    return (da > db) - (da < db);
}

static inline int djlc_cmp_double_desc(const void* a, const void* b) {
    double da = *(const double*)a;
    double db = *(const double*)b;
    return (db > da) - (db < da);
}

/* ========== Pointer to Int (for int** sorting) ========== */

/* Sort array of int pointers by first element */
static inline int djlc_cmp_int_ptr_first(const void* a, const void* b) {
    int ia = **(const int**)a;
    int ib = **(const int**)b;
    return (ia > ib) - (ia < ib);
}

#endif /* DJLC_COMPARE_H */

