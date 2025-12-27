/**
 * danieRTOS - Type Definitions
 *
 * Copyright (c) 2025 Danny Jiang <djiang.tw@gmail.com>
 * SPDX-License-Identifier: MIT
 */

#ifndef DANIERTOS_TYPES_H
#define DANIERTOS_TYPES_H

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

#include "config.h"

/* -----------------------------------------------------------------------------
 * Architecture-specific types
 * ---------------------------------------------------------------------------*/

#if CONFIG_RISCV_XLEN == 64
    typedef uint64_t reg_t;     /* Register size */
    typedef int64_t  sreg_t;    /* Signed register */
    #define REG_FMT  "0x%016lx"
#else
    typedef uint32_t reg_t;
    typedef int32_t  sreg_t;
    #define REG_FMT  "0x%08x"
#endif

/* -----------------------------------------------------------------------------
 * Kernel types
 * ---------------------------------------------------------------------------*/

/** Tick count type (64-bit to avoid overflow) */
typedef uint64_t tick_t;

/** Task priority type */
typedef uint8_t priority_t;

/** Task function pointer */
typedef void (*task_func_t)(void *arg);

/** Task state */
typedef enum {
    TASK_STATE_READY    = 0,
    TASK_STATE_RUNNING  = 1,
    TASK_STATE_BLOCKED  = 2,
    TASK_STATE_SUSPENDED = 3,
    TASK_STATE_DELETED  = 4
} task_state_t;

/** Wake reason (why a blocked task was woken up) */
typedef enum {
    WAKE_REASON_NONE     = 0,
    WAKE_REASON_SIGNALED = 1,   /* Resource became available */
    WAKE_REASON_TIMEOUT  = 2    /* Timeout expired */
} wake_reason_t;

/* -----------------------------------------------------------------------------
 * Forward declarations
 * ---------------------------------------------------------------------------*/

typedef struct tcb tcb_t;
typedef struct semaphore semaphore_t;
typedef struct mutex mutex_t;
typedef struct queue queue_t;

/* -----------------------------------------------------------------------------
 * Constants
 * ---------------------------------------------------------------------------*/

/** Wait forever (infinite timeout) */
#define WAIT_FOREVER    UINT32_MAX

/** No wait (try and return immediately) */
#define NO_WAIT         0

/* -----------------------------------------------------------------------------
 * Utility macros
 * ---------------------------------------------------------------------------*/

#define ARRAY_SIZE(arr)     (sizeof(arr) / sizeof((arr)[0]))
#define ALIGN_UP(x, a)      (((x) + (a) - 1) & ~((a) - 1))
#define ALIGN_DOWN(x, a)    ((x) & ~((a) - 1))

#define MIN(a, b)           ((a) < (b) ? (a) : (b))
#define MAX(a, b)           ((a) > (b) ? (a) : (b))

/* -----------------------------------------------------------------------------
 * Freestanding string functions (no libc)
 * ---------------------------------------------------------------------------*/

static inline void *memcpy(void *dest, const void *src, size_t n)
{
    uint8_t *d = (uint8_t *)dest;
    const uint8_t *s = (const uint8_t *)src;
    while (n--) *d++ = *s++;
    return dest;
}

static inline void *memset(void *s, int c, size_t n)
{
    uint8_t *p = (uint8_t *)s;
    while (n--) *p++ = (uint8_t)c;
    return s;
}

static inline int memcmp(const void *s1, const void *s2, size_t n)
{
    const uint8_t *p1 = (const uint8_t *)s1;
    const uint8_t *p2 = (const uint8_t *)s2;
    while (n--) {
        if (*p1 != *p2) return *p1 - *p2;
        p1++; p2++;
    }
    return 0;
}

#endif /* DANIERTOS_TYPES_H */

