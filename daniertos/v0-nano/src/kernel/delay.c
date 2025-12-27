/**
 * danieRTOS - Delay Implementation
 *
 * Copyright (c) 2025 Danny Jiang <djiang.tw@gmail.com>
 * SPDX-License-Identifier: MIT
 */

#include "daniertos.h"

/* -----------------------------------------------------------------------------
 * Delay List
 *
 * Tasks waiting for a delay are kept in a sorted list by wake_tick.
 * This makes tick_increment() efficient - just check the head.
 * ---------------------------------------------------------------------------*/

static tcb_t *g_delay_list = NULL;

/* -----------------------------------------------------------------------------
 * Internal Functions
 * ---------------------------------------------------------------------------*/

void delay_add(tcb_t *task, tick_t wake_tick)
{
    task->wake_tick = wake_tick;
    task->wake_reason = WAKE_REASON_NONE;

    /* Insert in sorted order */
    tcb_t **pp = &g_delay_list;
    while (*pp != NULL && (*pp)->wake_tick <= wake_tick) {
        pp = &(*pp)->next;
    }
    task->next = *pp;
    *pp = task;
}

void delay_check_wakeups(tick_t current_tick)
{
    /* Wake all tasks whose delay has expired */
    while (g_delay_list != NULL && g_delay_list->wake_tick <= current_tick) {
        tcb_t *task = g_delay_list;
        g_delay_list = task->next;
        task->next = NULL;

        task->wake_reason = WAKE_REASON_TIMEOUT;
        task_make_ready(task);
    }
}

/* -----------------------------------------------------------------------------
 * Public API
 * ---------------------------------------------------------------------------*/

void delay_ticks(tick_t ticks)
{
    if (ticks == 0) {
        /* Just yield if delay is 0 */
        task_yield();
        return;
    }

    reg_t state = critical_enter();

    tcb_t *current = task_get_current();
    tick_t wake_tick = tick_get() + ticks;

    /* Add to delay list and block */
    delay_add(current, wake_tick);
    task_block(current, NULL);  /* blocked_on = NULL means waiting on delay */

    critical_exit(state);

    /* Trigger context switch */
    task_yield();
}

void delay_ms(uint32_t ms)
{
    delay_ticks(tick_from_ms(ms));
}

void delay_until(tick_t wake_tick)
{
    tick_t now = tick_get();

    if (wake_tick <= now) {
        /* Already past wake time */
        return;
    }

    delay_ticks(wake_tick - now);
}

