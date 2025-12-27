/**
 *
 * SPDX-License-Identifier: MIT
 * Copyright (c) 2025 Danny Jiang
 * https://github.com/djiangtw/djiang-oss-public

 * utils.h - Common Utility Functions
 *
 * Frequently used utility functions for LeetCode problems.
 * Includes: swap, min, max, abs, clamp, etc.
 *
 * Author: Danny Jiang
 * Created: 2025-12-17
 */

#ifndef DJLC_UTILS_H
#define DJLC_UTILS_H

#include <stdint.h>

/* ========== Swap Functions ========== */

static inline void djlc_swap_int(int* a, int* b) {
    int t = *a;
    *a = *b;
    *b = t;
}

static inline void djlc_swap_long(long* a, long* b) {
    long t = *a;
    *a = *b;
    *b = t;
}

static inline void djlc_swap_char(char* a, char* b) {
    char t = *a;
    *a = *b;
    *b = t;
}

static inline void djlc_swap_ptr(void** a, void** b) {
    void* t = *a;
    *a = *b;
    *b = t;
}

/* XOR swap (no temp variable, same type only) */
static inline void djlc_swap_int_xor(int* a, int* b) {
    if (a != b) {
        *a ^= *b;
        *b ^= *a;
        *a ^= *b;
    }
}

/* ========== Min/Max Functions ========== */

static inline int djlc_min(int a, int b) {
    return a < b ? a : b;
}

static inline int djlc_max(int a, int b) {
    return a > b ? a : b;
}

static inline long djlc_min_long(long a, long b) {
    return a < b ? a : b;
}

static inline long djlc_max_long(long a, long b) {
    return a > b ? a : b;
}

static inline double djlc_min_double(double a, double b) {
    return a < b ? a : b;
}

static inline double djlc_max_double(double a, double b) {
    return a > b ? a : b;
}

/* Min/Max of three */
static inline int djlc_min3(int a, int b, int c) {
    return djlc_min(djlc_min(a, b), c);
}

static inline int djlc_max3(int a, int b, int c) {
    return djlc_max(djlc_max(a, b), c);
}

/* ========== Absolute Value ========== */

static inline int djlc_abs(int a) {
    return a < 0 ? -a : a;
}

static inline long djlc_abs_long(long a) {
    return a < 0 ? -a : a;
}

/* ========== Clamp ========== */

static inline int djlc_clamp(int val, int lo, int hi) {
    return val < lo ? lo : (val > hi ? hi : val);
}

static inline long djlc_clamp_long(long val, long lo, long hi) {
    return val < lo ? lo : (val > hi ? hi : val);
}

/* ========== Integer Division (ceiling) ========== */

static inline int djlc_div_ceil(int a, int b) {
    return (a + b - 1) / b;
}

/* ========== GCD / LCM ========== */

static inline int djlc_gcd(int a, int b) {
    while (b) {
        int t = b;
        b = a % b;
        a = t;
    }
    return a;
}

static inline int djlc_lcm(int a, int b) {
    return a / djlc_gcd(a, b) * b;  /* avoid overflow */
}

/* ========== Power (modular) ========== */

/* Compute base^exp % mod using binary exponentiation */
static inline long djlc_pow_mod(long base, long exp, long mod) {
    long result = 1;
    base %= mod;
    while (exp > 0) {
        if (exp & 1) result = result * base % mod;
        base = base * base % mod;
        exp >>= 1;
    }
    return result;
}

/* ========== Reverse Array ========== */

static inline void djlc_reverse_int(int* arr, int n) {
    for (int i = 0, j = n - 1; i < j; i++, j--) {
        djlc_swap_int(&arr[i], &arr[j]);
    }
}

static inline void djlc_reverse_char(char* arr, int n) {
    for (int i = 0, j = n - 1; i < j; i++, j--) {
        djlc_swap_char(&arr[i], &arr[j]);
    }
}

#endif /* DJLC_UTILS_H */

