/**
 * danieRTOS - Scheduler Implementation
 *
 * Copyright (c) 2025 Danny Jiang <djiang.tw@gmail.com>
 * SPDX-License-Identifier: MIT
 *
 * Priority-based preemptive scheduler with round-robin within same priority.
 */

#include "daniertos.h"

/* -----------------------------------------------------------------------------
 * State
 * ---------------------------------------------------------------------------*/

/* Ready queues - one per priority level */
static tcb_t *g_ready_queue[CONFIG_MAX_PRIORITY];

/* Scheduler state */
static sched_state_t g_sched_state = SCHED_STATE_STOPPED;

/* ISR nesting counter */
static volatile uint32_t g_isr_nesting = 0;

/* Context switch pending flag */
static volatile bool g_switch_pending = false;

/* External declarations from task.c */
extern void task_set_current(tcb_t *task);
extern tcb_t *task_get_idle(void);

/* -----------------------------------------------------------------------------
 * Ready Queue Management
 * ---------------------------------------------------------------------------*/

void sched_add_ready(tcb_t *task)
{
    if (task == NULL || task->state == TASK_STATE_DELETED) {
        return;
    }

    priority_t prio = task->priority;
    if (prio >= CONFIG_MAX_PRIORITY) {
        prio = CONFIG_MAX_PRIORITY - 1;
    }

    /* Add to end of queue for round-robin */
    tcb_t **pp = &g_ready_queue[prio];
    while (*pp != NULL) {
        pp = &(*pp)->next;
    }
    *pp = task;
    task->next = NULL;
    task->state = TASK_STATE_READY;
}

void sched_remove_ready(tcb_t *task)
{
    if (task == NULL) {
        return;
    }

    priority_t prio = task->priority;
    if (prio >= CONFIG_MAX_PRIORITY) {
        prio = CONFIG_MAX_PRIORITY - 1;
    }

    tcb_t **pp = &g_ready_queue[prio];
    while (*pp != NULL) {
        if (*pp == task) {
            *pp = task->next;
            task->next = NULL;
            return;
        }
        pp = &(*pp)->next;
    }

    /* Task might be in a different priority queue due to priority inheritance */
    for (int p = 0; p < CONFIG_MAX_PRIORITY; p++) {
        pp = &g_ready_queue[p];
        while (*pp != NULL) {
            if (*pp == task) {
                *pp = task->next;
                task->next = NULL;
                return;
            }
            pp = &(*pp)->next;
        }
    }
}

/* -----------------------------------------------------------------------------
 * Scheduler Core
 * ---------------------------------------------------------------------------*/

/**
 * Select highest priority ready task
 */
static tcb_t *select_next_task(void)
{
    /* Search from highest to lowest priority */
    for (int prio = CONFIG_MAX_PRIORITY - 1; prio >= 0; prio--) {
        if (g_ready_queue[prio] != NULL) {
            return g_ready_queue[prio];
        }
    }

    /* No ready task - return idle task */
    return task_get_idle();
}

void sched_init(void)
{
    for (int i = 0; i < CONFIG_MAX_PRIORITY; i++) {
        g_ready_queue[i] = NULL;
    }
    g_sched_state = SCHED_STATE_STOPPED;
    g_isr_nesting = 0;
    g_switch_pending = false;
}

void sched_start(void)
{
    /* Select first task */
    tcb_t *task = select_next_task();
    KERNEL_ASSERT(task != NULL);

    task->state = TASK_STATE_RUNNING;
    task_set_current(task);

    g_sched_state = SCHED_STATE_RUNNING;

    KERNEL_DEBUG("Starting scheduler, first task: %s", task->name);

    /* Initialize timer (sets mtimecmp and enables MIE_MTIE in mie) */
    timer_init();

    /* Note: We don't call interrupts_enable() here.
     * The initial task's mstatus has MPIE=1, so mret will enable
     * interrupts automatically when we jump to trap_exit. */

    /* Load context and jump to first task */
    /* This is done by loading sp and calling trap_exit which does mret */
    asm volatile (
        "mv     sp, %0      \n"
        "j      trap_exit   \n"
        :
        : "r"(task->sp)
    );

    /* Never reached */
    __builtin_unreachable();
}

sched_state_t sched_get_state(void)
{
    return g_sched_state;
}

bool sched_in_isr(void)
{
    return g_isr_nesting > 0;
}

void sched_request_switch(void)
{
    g_switch_pending = true;
}

reg_t *sched_schedule(reg_t *current_sp)
{
    tcb_t *current = task_get_current();
    tcb_t *next;

    /* Save current SP */
    if (current != NULL) {
        current->sp = current_sp;

        /* Move current to end of its priority queue (round-robin) */
        if (current->state == TASK_STATE_RUNNING) {
            current->state = TASK_STATE_READY;
            sched_remove_ready(current);
            sched_add_ready(current);
        }
    }

    /* Select next task */
    next = select_next_task();

    if (next != current) {
        KERNEL_DEBUG("Switch: %s -> %s",
                     current ? current->name : "(none)",
                     next->name);
    }

    /* Update state */
    sched_remove_ready(next);
    next->state = TASK_STATE_RUNNING;
    task_set_current(next);

    g_switch_pending = false;

    return next->sp;
}

reg_t *sched_yield(reg_t *current_sp)
{
    return sched_schedule(current_sp);
}

/* -----------------------------------------------------------------------------
 * ISR Nesting
 * ---------------------------------------------------------------------------*/

void sched_enter_isr(void)
{
    g_isr_nesting++;
}

void sched_exit_isr(void)
{
    if (g_isr_nesting > 0) {
        g_isr_nesting--;
    }
}

bool sched_switch_pending(void)
{
    return g_switch_pending;
}
