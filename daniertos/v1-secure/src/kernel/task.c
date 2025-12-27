/**
 * danieRTOS v1.x - Task Management Implementation
 *
 * Copyright (c) 2025 Danny Jiang <djiang.tw@gmail.com>
 * SPDX-License-Identifier: MIT
 *
 * v1.x: Added dual stack support for M+U mode
 */

#include "daniertos.h"
#include "config.h"

/* -----------------------------------------------------------------------------
 * State
 * ---------------------------------------------------------------------------*/

/* Task pool */
static tcb_t g_tasks[CONFIG_MAX_TASKS];
static uint8_t g_task_count = 0;

/* Current running task */
static tcb_t *g_current_task = NULL;

/* Idle task (runs in M-mode) */
static tcb_t *g_idle_task = NULL;
static uint8_t g_idle_stack[CONFIG_IDLE_STACK_SIZE] __attribute__((aligned(16)));

#if CONFIG_USER_MODE
/* Kernel stack pool for user tasks */
static uint8_t g_kernel_stacks[CONFIG_MAX_TASKS][CONFIG_KERNEL_STACK_SIZE]
    __attribute__((aligned(16)));
#endif

/* -----------------------------------------------------------------------------
 * Idle Task
 * ---------------------------------------------------------------------------*/

static void idle_task_func(void *arg)
{
    (void)arg;
    while (1) {
        /* Wait for interrupt */
        asm volatile ("wfi");
    }
}

/* -----------------------------------------------------------------------------
 * Stack Initialization
 *
 * v1.x: Dual stack architecture
 *
 * For U-mode tasks:
 *   - User stack: where task runs (sp points here in U-mode)
 *   - Kernel stack: where trap handler runs (mscratch points here)
 *
 * Initial context frame (on kernel stack for U-mode, user stack for M-mode):
 *   [0]  x0 (zero) - placeholder
 *   [1]  ra        - return address (task_exit_handler)
 *   [2]  sp        - user stack pointer (for U-mode tasks)
 *   [3]  gp
 *   ...
 *   [10] a0        - task argument
 *   ...
 *   [31] t6
 *   [32] mepc      - entry point (task function)
 *   [33] mstatus   - MPP=U for user tasks, MPP=M for kernel tasks
 * ---------------------------------------------------------------------------*/

/* Handler for when a task returns (shouldn't happen) */
static void task_exit_handler(void)
{
    uart_printf("Task %s exited unexpectedly!\n", g_current_task->name);
    task_delete(g_current_task);
    while (1) { asm volatile ("wfi"); }
}

/**
 * Initialize stack for a task
 *
 * @param stack_top     Top of user stack
 * @param func          Task entry point
 * @param arg           Task argument
 * @param privilege     TASK_PRIV_U or TASK_PRIV_M
 * @return              Initial stack pointer (points to context frame)
 */
static reg_t *stack_init(reg_t *stack_top, task_func_t func, void *arg,
                         uint8_t privilege)
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

#if CONFIG_USER_MODE
    if (privilege == TASK_PRIV_U) {
        /* U-mode task: MPP=0 (U-mode), MPIE=1 (enable interrupts on mret) */
        sp[33] = MSTATUS_MPIE | MSTATUS_MPP_U;
    } else {
        /* M-mode task: MPP=3 (M-mode), MPIE=1 */
        sp[33] = MSTATUS_MPIE | MSTATUS_MPP_M;
    }
#else
    (void)privilege;
    /* v0 compatibility: all tasks run in M-mode */
    sp[33] = MSTATUS_MPIE | MSTATUS_MPP_M;
#endif

    return sp;
}

/* -----------------------------------------------------------------------------
 * Task API Implementation
 * ---------------------------------------------------------------------------*/

void task_init(void)
{
    memset(g_tasks, 0, sizeof(g_tasks));
    g_task_count = 0;
    g_current_task = NULL;

    /* Create idle task (runs in M-mode, uses WFI) */
    g_idle_task = task_create_priv("idle", idle_task_func, NULL, 0,
                                   g_idle_stack, sizeof(g_idle_stack),
                                   TASK_PRIV_M);
    KERNEL_ASSERT(g_idle_task != NULL);
}

/**
 * Create a task with specified privilege level (v1.x internal API)
 */
tcb_t *task_create_priv(const char *name,
                        task_func_t func,
                        void *arg,
                        priority_t priority,
                        void *stack,
                        size_t stack_size,
                        uint8_t privilege)
{
    if (g_task_count >= CONFIG_MAX_TASKS) {
        return NULL;
    }

    reg_t state = critical_enter();

    /* Find free TCB slot */
    tcb_t *task = NULL;
    int slot_idx = -1;
    for (int i = 0; i < CONFIG_MAX_TASKS; i++) {
        if (g_tasks[i].state == TASK_STATE_DELETED ||
            (g_tasks[i].stack_base == NULL && g_tasks[i].name == NULL)) {
            task = &g_tasks[i];
            slot_idx = i;
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
    task->wake_tick = 0;
    task->wake_reason = WAKE_REASON_NONE;
    task->blocked_on = NULL;
    task->next = NULL;

    /* User stack setup */
    task->stack_base = (reg_t *)stack;
    task->stack_size = stack_size;

    /* Align user stack top to 16 bytes */
    reg_t *user_stack_top = (reg_t *)((reg_t)stack + stack_size);
    user_stack_top = (reg_t *)((reg_t)user_stack_top & ~0xF);

#if CONFIG_USER_MODE
    task->privilege = privilege;

    if (privilege == TASK_PRIV_U) {
        /* -- U-mode task: set up dual stacks -- */

        /* Kernel stack for this task */
        task->kernel_stack_base = (reg_t *)g_kernel_stacks[slot_idx];
        task->kernel_stack_size = CONFIG_KERNEL_STACK_SIZE;

        reg_t *kernel_stack_top = (reg_t *)((reg_t)task->kernel_stack_base +
                                            CONFIG_KERNEL_STACK_SIZE);
        kernel_stack_top = (reg_t *)((reg_t)kernel_stack_top & ~0xF);

        /* Initialize context on kernel stack (trap handler will use this) */
        task->sp = stack_init(kernel_stack_top, func, arg, TASK_PRIV_U);

        /* Set user SP in context frame (will be restored on mret) */
        task->sp[2] = (reg_t)user_stack_top;

        /* kernel_sp points to top of kernel stack (for mscratch) */
        task->kernel_sp = kernel_stack_top;
    } else {
        /* -- M-mode task: single stack (like v0) -- */
        task->kernel_stack_base = NULL;
        task->kernel_stack_size = 0;
        task->kernel_sp = NULL;

        task->sp = stack_init(user_stack_top, func, arg, TASK_PRIV_M);
    }
#else
    /* v0 compatibility: all tasks use single stack in M-mode */
    (void)privilege;
    task->sp = stack_init(user_stack_top, func, arg, TASK_PRIV_M);
#endif

    /* Add to ready queue */
    sched_add_ready(task);

    critical_exit(state);

    KERNEL_DEBUG("Created task '%s' (id=%d, prio=%d, priv=%s)",
                 name, task->id, priority,
                 privilege == TASK_PRIV_U ? "U" : "M");

    return task;
}

/**
 * Create a user-mode task (default for v1.x)
 */
tcb_t *task_create(const char *name,
                   task_func_t func,
                   void *arg,
                   priority_t priority,
                   void *stack,
                   size_t stack_size)
{
#if CONFIG_USER_MODE
    /* Default to U-mode for user tasks */
    return task_create_priv(name, func, arg, priority, stack, stack_size,
                            TASK_PRIV_U);
#else
    return task_create_priv(name, func, arg, priority, stack, stack_size,
                            TASK_PRIV_M);
#endif
}

void task_delete(tcb_t *task)
{
    if (task == NULL) {
        task = g_current_task;
    }

    reg_t state = critical_enter();

    sched_remove_ready(task);
    task->state = TASK_STATE_DELETED;

    critical_exit(state);

    if (task == g_current_task) {
        task_yield();
    }
}

void task_exit(void)
{
    /*
     * Called from syscall handler when user task calls sys_exit().
     *
     * We only mark the task as deleted here. We do NOT call task_yield()
     * because that would execute ecall from within the syscall handler
     * (M-mode), causing nested trap handling issues.
     *
     * The syscall handler will call sched_schedule() after this returns,
     * which will switch to another ready task.
     */
    reg_t state = critical_enter();

    tcb_t *task = g_current_task;
    sched_remove_ready(task);
    task->state = TASK_STATE_DELETED;

    critical_exit(state);

    /* Syscall handler will call sched_schedule() to switch task */
}

void task_delay(tick_t ticks)
{
    if (ticks == 0) {
        return;
    }

    reg_t state = critical_enter();

    tcb_t *task = g_current_task;
    task->wake_tick = tick_get() + ticks;
    task->state = TASK_STATE_BLOCKED;
    task->wake_reason = WAKE_REASON_TIMEOUT;
    sched_remove_ready(task);

    /* Add to delay list (handled by tick_increment) */
    sched_add_delayed(task);

    critical_exit(state);

    /* Will be rescheduled by syscall handler */
}

tcb_t *task_get_current(void)
{
    return g_current_task;
}

void task_set_current(tcb_t *task)
{
    g_current_task = task;
}

tcb_t *task_get_idle(void)
{
    return g_idle_task;
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
    if (task == NULL) {
        task = g_current_task;
    }

    reg_t state = critical_enter();

    sched_remove_ready(task);
    task->state = TASK_STATE_SUSPENDED;

    critical_exit(state);

    if (task == g_current_task) {
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
        task = g_current_task;
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
    task->state = TASK_STATE_READY;
    task->blocked_on = NULL;
    sched_add_ready(task);
}

void task_block(tcb_t *task, void *blocked_on)
{
    task->state = TASK_STATE_BLOCKED;
    task->blocked_on = blocked_on;
    sched_remove_ready(task);
}

