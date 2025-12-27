/**
 * danieRTOS v2.x - Task Management Implementation (SMP Edition)
 *
 * Copyright (c) 2025 Danny Jiang <djiang.tw@gmail.com>
 * SPDX-License-Identifier: MIT
 */

#include "daniertos.h"
#include "smp.h"
#include "spinlock.h"

/* -----------------------------------------------------------------------------
 * State
 * ---------------------------------------------------------------------------*/

/* Task pool */
static tcb_t g_tasks[CONFIG_MAX_TASKS];
static uint8_t g_task_count = 0;

/* Per-core idle tasks - use CONFIG_MAX_CORES for static allocation */
static tcb_t *g_idle_tasks[CONFIG_MAX_CORES];
static uint8_t g_idle_stacks[CONFIG_MAX_CORES][CONFIG_IDLE_STACK_SIZE]
    __attribute__((aligned(16)));

/* External scheduler lock */
extern spinlock_t g_sched_lock;

/* -----------------------------------------------------------------------------
 * Idle Task
 * ---------------------------------------------------------------------------*/

static void idle_task_func(void *arg)
{
    (void)arg;
    /* Minimal idle loop - just wait for interrupts */
    while (1) {
        asm volatile ("wfi");
    }
}

/* -----------------------------------------------------------------------------
 * Stack Initialization
 *
 * Initial stack frame (grows downward):
 *   [0]  x0 (zero) - placeholder
 *   [1]  ra        - return address (task_exit_handler)
 *   [2]  sp        - placeholder
 *   [3]  gp
 *   ...
 *   [10] a0        - task argument
 *   ...
 *   [31] t6
 *   [32] mepc      - entry point (task function)
 *   [33] mstatus   - interrupts enabled
 * ---------------------------------------------------------------------------*/

/* Handler for when a task returns (shouldn't happen) */
static void task_exit_handler(void)
{
    cpu_t *cpu = smp_get_cpu();
    uart_printf("[Core %d] Task %s exited unexpectedly!\n",
                cpu->hartid, cpu->current_task->name);
    task_delete(cpu->current_task);
    while (1) { asm volatile ("wfi"); }
}

static reg_t *stack_init(reg_t *stack_top, task_func_t func, void *arg)
{
    /* Make room for context frame */
    reg_t *sp = stack_top - 34;  /* 32 GP regs + mepc + mstatus */

    /* Clear all registers */
    for (int i = 0; i < 34; i++) {
        sp[i] = 0;
    }

    /* Set up initial context */
    sp[1]  = (reg_t)task_exit_handler;  /* ra - return address */
    sp[2]  = (reg_t)stack_top;          /* sp - stack pointer after context restore */
    sp[10] = (reg_t)arg;                /* a0 - task argument */
    sp[32] = (reg_t)func;               /* mepc - task entry point */
    sp[33] = MSTATUS_MPIE | (3UL << 11); /* mstatus - MPP=M, MPIE=1 */

    return sp;
}

/* -----------------------------------------------------------------------------
 * Task API Implementation
 * ---------------------------------------------------------------------------*/

void task_init(void)
{
    memset(g_tasks, 0, sizeof(g_tasks));
    g_task_count = 0;

    /* Create per-core idle tasks */
    static const char *idle_names[] = {"idle0", "idle1", "idle2", "idle3",
                                        "idle4", "idle5", "idle6", "idle7"};
    for (uint32_t i = 0; i < CONFIG_NUM_CORES; i++) {
        g_idle_tasks[i] = task_create(
            idle_names[i],
            idle_task_func,
            NULL,
            0,  /* lowest priority */
            g_idle_stacks[i],
            CONFIG_IDLE_STACK_SIZE,
            (1U << i)  /* Only run on core i */
        );
        KERNEL_ASSERT(g_idle_tasks[i] != NULL);

        /* Remove idle from ready queue - it's only used when nothing else is ready */
        sched_remove_ready(g_idle_tasks[i]);
        g_idle_tasks[i]->state = TASK_STATE_READY;
    }

    uart_printf("[Core 0] Created %d idle tasks\n", CONFIG_NUM_CORES);
}

tcb_t *task_create(const char *name,
                   task_func_t func,
                   void *arg,
                   priority_t priority,
                   void *stack,
                   size_t stack_size,
                   uint32_t affinity)
{
    if (g_task_count >= CONFIG_MAX_TASKS) {
        return NULL;
    }

    reg_t state = critical_enter();

    /* Find free TCB slot */
    tcb_t *task = NULL;
    for (int i = 0; i < CONFIG_MAX_TASKS; i++) {
        if (g_tasks[i].state == TASK_STATE_DELETED ||
            (g_tasks[i].stack_base == NULL && g_tasks[i].name == NULL)) {
            task = &g_tasks[i];
            break;
        }
    }

    if (task == NULL) {
        critical_exit(state);
        return NULL;
    }

    /* Initialize TCB */
    task->id = g_task_count++;
    task->name = name;
    task->priority = priority;
    task->base_priority = priority;
    task->state = TASK_STATE_READY;
    task->affinity_mask = (affinity == 0) ? CONFIG_CORE_ANY : affinity;
    task->wake_tick = 0;
    task->wake_reason = WAKE_REASON_NONE;
    task->blocked_on = NULL;
    task->next = NULL;
    task->delay_next = NULL;

    /* Stack setup */
    task->stack_base = (reg_t *)stack;
    task->stack_size = stack_size;

    /* Align stack top to 16 bytes */
    reg_t *stack_top = (reg_t *)((reg_t)stack + stack_size);
    stack_top = (reg_t *)((reg_t)stack_top & ~0xF);

    /* Initialize stack with context */
    task->sp = stack_init(stack_top, func, arg);

    /* Add to ready queue */
    sched_add_ready(task);

    critical_exit(state);

    KERNEL_DEBUG("Created task '%s' (id=%d, prio=%d, affinity=0x%x)",
                 name, task->id, priority, task->affinity_mask);

    return task;
}

void task_delete(tcb_t *task)
{
    cpu_t *cpu = smp_get_cpu();

    if (task == NULL) {
        task = cpu->current_task;
    }

    reg_t state = critical_enter();

    sched_remove_ready(task);
    task->state = TASK_STATE_DELETED;

    critical_exit(state);

    if (task == cpu->current_task) {
        task_yield();
    }
}

tcb_t *task_get_current(void)
{
    cpu_t *cpu = smp_get_cpu();
    return cpu->current_task;
}

void task_set_current(tcb_t *task)
{
    cpu_t *cpu = smp_get_cpu();
    cpu->current_task = task;
}

tcb_t *task_get_idle_for_core(uint64_t hartid)
{
    if (hartid >= CONFIG_NUM_CORES) {
        return g_idle_tasks[0];
    }
    return g_idle_tasks[hartid];
}

void task_yield(void)
{
    sched_request_switch();
    /* Force an immediate context switch using ecall (environment call).
     * This will trap to trap_handler with mcause = 11 (ecall from M-mode) */
    asm volatile ("ecall");
}

void task_suspend(tcb_t *task)
{
    cpu_t *cpu = smp_get_cpu();

    if (task == NULL) {
        task = cpu->current_task;
    }

    reg_t state = critical_enter();

    sched_remove_ready(task);
    task->state = TASK_STATE_SUSPENDED;

    critical_exit(state);

    if (task == cpu->current_task) {
        task_yield();
    }
}

void task_resume(tcb_t *task)
{
    if (task == NULL || task->state != TASK_STATE_SUSPENDED) {
        return;
    }

    reg_t state = critical_enter();

    task_make_ready(task);

    critical_exit(state);
}

task_state_t task_get_state(tcb_t *task)
{
    return task->state;
}

void task_set_priority(tcb_t *task, priority_t priority)
{
    if (task == NULL) {
        cpu_t *cpu = smp_get_cpu();
        task = cpu->current_task;
    }

    reg_t state = critical_enter();

    task->base_priority = priority;
    task->priority = priority;

    /* Re-sort in ready queue if needed */
    if (task->state == TASK_STATE_READY) {
        sched_remove_ready(task);
        sched_add_ready(task);
    }

    critical_exit(state);
}

priority_t task_get_priority(tcb_t *task)
{
    return task->priority;
}

void task_make_ready(tcb_t *task)
{
    extern spinlock_t g_sched_lock;
    reg_t state = spinlock_lock_irqsave(&g_sched_lock);

    task->state = TASK_STATE_READY;
    task->blocked_on = NULL;
    sched_add_ready(task);

    spinlock_unlock_irqrestore(&g_sched_lock, state);
}

void task_block(tcb_t *task, void *blocked_on)
{
    extern spinlock_t g_sched_lock;
    reg_t state = spinlock_lock_irqsave(&g_sched_lock);

    task->state = TASK_STATE_BLOCKED;
    task->blocked_on = blocked_on;
    sched_remove_ready(task);

    spinlock_unlock_irqrestore(&g_sched_lock, state);
}

