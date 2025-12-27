/**
 * danieRTOS - Delay Implementation (SMP Edition)
 *
 * Copyright (c) 2025 Danny Jiang <djiang.tw@gmail.com>
 * SPDX-License-Identifier: MIT
 */

#include "daniertos.h"
#include "spinlock.h"

/* -----------------------------------------------------------------------------
 * Delay List
 *
 * Tasks waiting for a delay are kept in a sorted list by wake_tick.
 * This makes tick_increment() efficient - just check the head.
 * Protected by spinlock for SMP safety.
 * ---------------------------------------------------------------------------*/

static tcb_t *g_delay_list = NULL;
static spinlock_t g_delay_lock = SPINLOCK_INIT;

/* -----------------------------------------------------------------------------
 * Internal Functions
 * ---------------------------------------------------------------------------*/

/* Internal: add task to delay list (caller must hold g_delay_lock) */
static void delay_add_locked(tcb_t *task, tick_t wake_tick)
{
    task->wake_tick = wake_tick;
    task->wake_reason = WAKE_REASON_NONE;

    /* Insert in sorted order (using delay_next for delay list) */
    tcb_t **pp = &g_delay_list;
    while (*pp != NULL && (*pp)->wake_tick <= wake_tick) {
        pp = &(*pp)->delay_next;
    }
    task->delay_next = *pp;
    *pp = task;
}

void delay_check_wakeups(tick_t current_tick)
{
    reg_t state = spinlock_lock_irqsave(&g_delay_lock);

    /* Wake all tasks whose delay has expired */
    while (g_delay_list != NULL && g_delay_list->wake_tick <= current_tick) {
        tcb_t *task = g_delay_list;
        g_delay_list = task->delay_next;
        task->delay_next = NULL;

        task->wake_reason = WAKE_REASON_TIMEOUT;
        spinlock_unlock_irqrestore(&g_delay_lock, state);

        task_make_ready(task);
        state = spinlock_lock_irqsave(&g_delay_lock);
    }

    spinlock_unlock_irqrestore(&g_delay_lock, state);
}

/* -----------------------------------------------------------------------------
 * Public API
 * ---------------------------------------------------------------------------*/

void delay_add(tcb_t *task, tick_t wake_tick)
{
    reg_t state = spinlock_lock_irqsave(&g_delay_lock);
    delay_add_locked(task, wake_tick);
    spinlock_unlock_irqrestore(&g_delay_lock, state);
}

void delay_remove(tcb_t *task)
{
    reg_t state = spinlock_lock_irqsave(&g_delay_lock);

    /* Remove task from delay list if present (using delay_next) */
    tcb_t **pp = &g_delay_list;
    while (*pp != NULL) {
        if (*pp == task) {
            *pp = task->delay_next;
            task->delay_next = NULL;
            break;
        }
        pp = &(*pp)->delay_next;
    }

    spinlock_unlock_irqrestore(&g_delay_lock, state);
}

void delay_ticks(tick_t ticks)
{
    if (ticks == 0) {
        /* Just yield if delay is 0 */
        task_yield();
        return;
    }

    reg_t state = spinlock_lock_irqsave(&g_delay_lock);

    tcb_t *current = task_get_current();
    tick_t wake_tick = tick_get() + ticks;

    /* Add to delay list and block */
    delay_add_locked(current, wake_tick);
    task_block(current, NULL);  /* blocked_on = NULL means waiting on delay */

    spinlock_unlock_irqrestore(&g_delay_lock, state);

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

