/**
 * danieRTOS v2.x - Spinlock
 *
 * Copyright (c) 2025 Danny Jiang <djiang.tw@gmail.com>
 * SPDX-License-Identifier: MIT
 *
 * Simple spinlock implementation using RISC-V atomic instructions.
 * Uses AMOSWAP with acquire/release semantics.
 */

#ifndef DANIERTOS_SPINLOCK_H
#define DANIERTOS_SPINLOCK_H

#include "types.h"

/* -----------------------------------------------------------------------------
 * Spinlock Structure
 * ---------------------------------------------------------------------------*/

typedef struct spinlock {
    volatile uint32_t locked;
} spinlock_t;

#define SPINLOCK_INIT { .locked = 0 }

/* -----------------------------------------------------------------------------
 * Spinlock API
 * ---------------------------------------------------------------------------*/

/**
 * Initialize a spinlock
 */
static inline void spinlock_init(spinlock_t *lock)
{
    lock->locked = 0;
}

/**
 * Acquire a spinlock (busy-wait)
 * Uses AMOSWAP with acquire semantics
 */
static inline void spinlock_acquire(spinlock_t *lock)
{
    uint32_t tmp;
    asm volatile (
        "1:                             \n"
        "   li      %0, 1               \n"  /* tmp = 1 */
        "   amoswap.w.aq %0, %0, (%1)   \n"  /* swap(lock, tmp), acquire */
        "   bnez    %0, 1b              \n"  /* if old value != 0, retry */
        : "=&r"(tmp)
        : "r"(&lock->locked)
        : "memory"
    );
}

/**
 * Try to acquire a spinlock (non-blocking)
 * @return true if lock acquired, false otherwise
 */
static inline bool spinlock_try_acquire(spinlock_t *lock)
{
    uint32_t tmp;
    asm volatile (
        "   li      %0, 1               \n"
        "   amoswap.w.aq %0, %0, (%1)   \n"
        : "=&r"(tmp)
        : "r"(&lock->locked)
        : "memory"
    );
    return tmp == 0;
}

/**
 * Release a spinlock
 * Uses AMOSWAP with release semantics
 */
static inline void spinlock_release(spinlock_t *lock)
{
    asm volatile (
        "   amoswap.w.rl zero, zero, (%0)  \n"
        :
        : "r"(&lock->locked)
        : "memory"
    );
}

/**
 * Check if spinlock is held
 */
static inline bool spinlock_is_locked(spinlock_t *lock)
{
    return lock->locked != 0;
}

/* -----------------------------------------------------------------------------
 * Spinlock with IRQ Save/Restore
 *
 * These functions disable local interrupts before acquiring the lock,
 * preventing deadlock between interrupt handler and task.
 * ---------------------------------------------------------------------------*/

/**
 * Acquire spinlock and disable interrupts
 * @return Previous mstatus value (for restore)
 */
static inline reg_t spinlock_lock_irqsave(spinlock_t *lock)
{
    reg_t mstatus;
    
    /* Save and disable interrupts */
    asm volatile (
        "csrr   %0, mstatus         \n"
        "csrc   mstatus, 0x8        \n"  /* Clear MIE bit */
        : "=r"(mstatus)
        :
        : "memory"
    );
    
    /* Acquire lock */
    spinlock_acquire(lock);
    
    return mstatus;
}

/**
 * Release spinlock and restore interrupts
 * @param lock    Spinlock to release
 * @param mstatus Previous mstatus value (from irqsave)
 */
static inline void spinlock_unlock_irqrestore(spinlock_t *lock, reg_t mstatus)
{
    /* Release lock */
    spinlock_release(lock);
    
    /* Restore interrupt state */
    if (mstatus & 0x8) {
        asm volatile ("csrs mstatus, 0x8" ::: "memory");
    }
}

#endif /* DANIERTOS_SPINLOCK_H */

