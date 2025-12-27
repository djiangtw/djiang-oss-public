/**
 * danieRTOS - Critical Section API
 *
 * Copyright (c) 2025 Danny Jiang <djiang.tw@gmail.com>
 * SPDX-License-Identifier: MIT
 */

#ifndef DANIERTOS_CRITICAL_H
#define DANIERTOS_CRITICAL_H

#include "types.h"

/* -----------------------------------------------------------------------------
 * Critical Section API
 *
 * Critical sections protect shared data from concurrent access by:
 * 1. Disabling interrupts (prevents preemption)
 * 2. Supporting nesting (only re-enable on outermost exit)
 * ---------------------------------------------------------------------------*/

/**
 * Enter critical section
 *
 * Disables interrupts and returns the previous interrupt state.
 * Can be nested - interrupts only re-enabled when nesting count reaches 0.
 *
 * @return  Previous interrupt state (to be passed to critical_exit)
 */
reg_t critical_enter(void);

/**
 * Exit critical section
 *
 * Restores the interrupt state from before the corresponding critical_enter.
 *
 * @param state  State returned by critical_enter
 */
void critical_exit(reg_t state);

/* -----------------------------------------------------------------------------
 * Convenience Macros
 * ---------------------------------------------------------------------------*/

/**
 * CRITICAL_SECTION macro for scoped critical sections
 *
 * Usage:
 *   CRITICAL_SECTION {
 *       // protected code
 *   }
 */
#define CRITICAL_SECTION \
    for (reg_t __critical_state = critical_enter(), __once = 1; \
         __once; \
         critical_exit(__critical_state), __once = 0)

#endif /* DANIERTOS_CRITICAL_H */

