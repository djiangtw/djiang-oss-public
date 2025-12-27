/**
 * danieRTOS - Delay/Sleep API
 *
 * Copyright (c) 2025 Danny Jiang <djiang.tw@gmail.com>
 * SPDX-License-Identifier: MIT
 */

#ifndef DANIERTOS_DELAY_H
#define DANIERTOS_DELAY_H

#include "types.h"

/* -----------------------------------------------------------------------------
 * Delay API
 * ---------------------------------------------------------------------------*/

/**
 * Delay for specified number of ticks
 *
 * @param ticks  Number of ticks to delay
 */
void delay_ticks(tick_t ticks);

/**
 * Delay for specified number of milliseconds
 *
 * @param ms  Number of milliseconds to delay
 */
void delay_ms(uint32_t ms);

/**
 * Delay until a specific absolute tick
 *
 * @param wake_tick  Absolute tick to wake at
 */
void delay_until(tick_t wake_tick);

/* -----------------------------------------------------------------------------
 * Internal
 * ---------------------------------------------------------------------------*/

/**
 * Add task to delay list (for timeout blocking)
 */
void delay_add(tcb_t *task, tick_t wake_tick);

/**
 * Remove task from delay list (if woken by other means)
 */
void delay_remove(tcb_t *task);

/**
 * Check delay list and wake expired tasks
 * Called from tick_increment()
 */
void delay_check_wakeups(tick_t current_tick);

#endif /* DANIERTOS_DELAY_H */

