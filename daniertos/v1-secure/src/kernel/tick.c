/**
 * danieRTOS - System Tick Implementation
 *
 * Copyright (c) 2025 Danny Jiang <djiang.tw@gmail.com>
 * SPDX-License-Identifier: MIT
 */

#include "daniertos.h"

/* -----------------------------------------------------------------------------
 * State
 * ---------------------------------------------------------------------------*/

static volatile tick_t g_tick_count = 0;

/* -----------------------------------------------------------------------------
 * Public API
 * ---------------------------------------------------------------------------*/

void tick_init(void)
{
    g_tick_count = 0;
    /* Note: timer_init() is called by sched_start() to avoid
     * enabling timer interrupts before the scheduler is ready */
}

tick_t tick_get(void)
{
    tick_t ticks;

    /* Read atomically (disable interrupts briefly on 32-bit) */
    CRITICAL_SECTION {
        ticks = g_tick_count;
    }

    return ticks;
}

tick_t tick_from_ms(uint32_t ms)
{
    return (tick_t)ms * CONFIG_TICK_RATE_HZ / 1000;
}

uint32_t tick_to_ms(tick_t ticks)
{
    return (uint32_t)(ticks * 1000 / CONFIG_TICK_RATE_HZ);
}

/* -----------------------------------------------------------------------------
 * ISR Handler
 * ---------------------------------------------------------------------------*/

/* External declaration for delayed task check */
extern void sched_check_delayed(void);

void tick_increment(void)
{
    g_tick_count++;

    /* Check for tasks to wake from delay (v1.x) */
    sched_check_delayed();

    /* Check for tasks to wake from delay (v0 compatibility) */
    delay_check_wakeups(g_tick_count);

    /* Request context switch (preemption) */
    sched_request_switch();
}

