/**
 *
 * SPDX-License-Identifier: MIT
 * Copyright (c) 2025 Danny Jiang
 * https://github.com/djiangtw/djiang-oss-public

 * mod_math.h - Modular Arithmetic Utilities
 *
 * Common operations for problems requiring modular arithmetic.
 * Handles overflow safely using 64-bit intermediate calculations.
 *
 * Author: Danny Jiang
 * Created: 2025-12-24
 */

#ifndef DJLC_MOD_MATH_H
#define DJLC_MOD_MATH_H

#include <stdint.h>

/*============================================================================
 * COMMON MODULO CONSTANTS
 *===========================================================================*/

#define DJLC_MOD_1E9_7  1000000007LL  /* 10^9 + 7, most common */
#define DJLC_MOD_1E9_9  1000000009LL  /* 10^9 + 9 */
#define DJLC_MOD_998244353 998244353LL /* NTT-friendly prime */

/*============================================================================
 * BASIC MODULAR OPERATIONS
 *===========================================================================*/

/* Ensure result is in [0, mod) */
static inline long long djlc_mod(long long a, long long mod) {
    a %= mod;
    if (a < 0) a += mod;
    return a;
}

/* Modular addition: (a + b) % mod */
static inline long long djlc_mod_add(long long a, long long b, long long mod) {
    return djlc_mod(djlc_mod(a, mod) + djlc_mod(b, mod), mod);
}

/* Modular subtraction: (a - b) % mod */
static inline long long djlc_mod_sub(long long a, long long b, long long mod) {
    return djlc_mod(djlc_mod(a, mod) - djlc_mod(b, mod) + mod, mod);
}

/* Modular multiplication: (a * b) % mod */
static inline long long djlc_mod_mul(long long a, long long b, long long mod) {
    return djlc_mod(djlc_mod(a, mod) * djlc_mod(b, mod), mod);
}

/*============================================================================
 * MODULAR EXPONENTIATION
 *===========================================================================*/

/* Fast modular exponentiation: (base^exp) % mod */
static inline long long djlc_mod_pow(long long base, long long exp, long long mod) {
    long long result = 1;
    base = djlc_mod(base, mod);
    while (exp > 0) {
        if (exp & 1) {
            result = djlc_mod_mul(result, base, mod);
        }
        base = djlc_mod_mul(base, base, mod);
        exp >>= 1;
    }
    return result;
}

/*============================================================================
 * MODULAR INVERSE
 * Works only when mod is prime (uses Fermat's little theorem)
 *===========================================================================*/

/* Modular inverse: a^(-1) % mod, where mod is prime */
static inline long long djlc_mod_inv(long long a, long long mod) {
    return djlc_mod_pow(a, mod - 2, mod);
}

/* Modular division: (a / b) % mod, where mod is prime */
static inline long long djlc_mod_div(long long a, long long b, long long mod) {
    return djlc_mod_mul(a, djlc_mod_inv(b, mod), mod);
}

/*============================================================================
 * FACTORIAL AND COMBINATIONS (for nCr problems)
 *===========================================================================*/

/* Precompute factorials and inverse factorials for nCr queries */
typedef struct {
    long long *fact;     /* fact[i] = i! % mod */
    long long *inv_fact; /* inv_fact[i] = (i!)^(-1) % mod */
    int size;
    long long mod;
} djlc_mod_factorial_t;

static inline int djlc_mod_factorial_init(djlc_mod_factorial_t *mf, int n, long long mod) {
    mf->fact = (long long *)malloc((n + 1) * sizeof(long long));
    mf->inv_fact = (long long *)malloc((n + 1) * sizeof(long long));
    if (!mf->fact || !mf->inv_fact) {
        free(mf->fact);
        free(mf->inv_fact);
        return 0;
    }
    mf->size = n + 1;
    mf->mod = mod;
    
    /* Compute factorials */
    mf->fact[0] = 1;
    for (int i = 1; i <= n; i++) {
        mf->fact[i] = djlc_mod_mul(mf->fact[i - 1], i, mod);
    }
    
    /* Compute inverse factorials */
    mf->inv_fact[n] = djlc_mod_inv(mf->fact[n], mod);
    for (int i = n - 1; i >= 0; i--) {
        mf->inv_fact[i] = djlc_mod_mul(mf->inv_fact[i + 1], i + 1, mod);
    }
    
    return 1;
}

static inline void djlc_mod_factorial_free(djlc_mod_factorial_t *mf) {
    if (mf->fact) { free(mf->fact); mf->fact = NULL; }
    if (mf->inv_fact) { free(mf->inv_fact); mf->inv_fact = NULL; }
    mf->size = 0;
}

/* nCr = n! / (r! * (n-r)!) */
static inline long long djlc_mod_ncr(djlc_mod_factorial_t *mf, int n, int r) {
    if (r < 0 || r > n) return 0;
    return djlc_mod_mul(mf->fact[n],
                        djlc_mod_mul(mf->inv_fact[r], mf->inv_fact[n - r], mf->mod),
                        mf->mod);
}

/* nPr = n! / (n-r)! */
static inline long long djlc_mod_npr(djlc_mod_factorial_t *mf, int n, int r) {
    if (r < 0 || r > n) return 0;
    return djlc_mod_mul(mf->fact[n], mf->inv_fact[n - r], mf->mod);
}

#endif /* DJLC_MOD_MATH_H */

