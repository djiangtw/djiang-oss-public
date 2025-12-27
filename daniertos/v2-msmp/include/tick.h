/**
 * danieRTOS - System Tick API
 *
 * Copyright (c) 2025 Danny Jiang <djiang.tw@gmail.com>
 * SPDX-License-Identifier: MIT
 */

#ifndef DANIERTOS_TICK_H
#define DANIERTOS_TICK_H

#include "types.h"

/* -----------------------------------------------------------------------------
 * Tick API
 * ---------------------------------------------------------------------------*/

/**
 * Initialize tick system
 */
void tick_init(void);

/**
 * Get current tick count
 */
tick_t tick_get(void);

/**
 * Convert milliseconds to ticks
 */
tick_t tick_from_ms(uint32_t ms);

/**
 * Convert ticks to milliseconds
 */
uint32_t tick_to_ms(tick_t ticks);

/* -----------------------------------------------------------------------------
 * Internal (called from timer ISR)
 * ---------------------------------------------------------------------------*/

/**
 * Increment tick and check for wakeups
 * Called from timer interrupt handler
 */
void tick_increment(void);

#endif /* DANIERTOS_TICK_H */

