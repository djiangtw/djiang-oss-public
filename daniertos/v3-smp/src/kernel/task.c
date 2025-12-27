/**
 * danieRTOS v3.x - Task Management Implementation (SMP + User Mode)
 *
 * Copyright (c) 2025 Danny Jiang <djiang.tw@gmail.com>
 * SPDX-License-Identifier: MIT
 *
 * v3.x: Combines SMP (v2.x) with User Mode (v1.x)
 *   - Per-Task Kernel Stack for trap handling
 *   - Per-Task User Stack for U-mode execution
 *   - Dual-stack context switch support
 */

#include "daniertos.h"
#include "smp.h"
#include "spinlock.h"
#include "pmp.h"

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

#if CONFIG_USER_MODE
/* User task stack pool (static allocation for simplicity) */
#define USER_KSTACK_SIZE    CONFIG_KSTACK_SIZE  /* 4KB kernel stack */
#define USER_USTACK_SIZE    CONFIG_USTACK_SIZE  /* 8KB user stack */
#define MAX_USER_TASKS      8

static uint8_t g_user_kstacks[MAX_USER_TASKS][USER_KSTACK_SIZE]
    __attribute__((aligned(16)));
/* User stacks must be in .user_data section for U-mode access via PMP */
static uint8_t g_user_ustacks[MAX_USER_TASKS][USER_USTACK_SIZE]
    __attribute__((aligned(16), section(".user_data")));
static uint8_t g_user_stack_used[MAX_USER_TASKS];
#endif

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

/* Handler for when a task returns (cleanup and wait for reschedule) */
static void task_exit_handler(void)
{
    cpu_t *cpu = smp_get_cpu();
    tcb_t *task = cpu->current_task;

    /* Clean exit - task completed */
    task_delete(task);

    /* Wait for next timer interrupt to trigger reschedule */
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
        /* U-mode task: MPP=0 (U-mode), MPIE=1 (enable interrupts on mret)
         * Also set UXL=2 (64-bit) for RV64 */
        sp[33] = MSTATUS_MPIE | MSTATUS_MPP_U | MSTATUS_UXL_64 | MSTATUS_SXL_64;
        /* tp[4] = 0 for U-mode (user TLS, set by task) */
    } else {
        /* M-mode task: MPP=3 (M-mode), MPIE=1 */
        sp[33] = MSTATUS_MPIE | MSTATUS_MPP_M | MSTATUS_UXL_64 | MSTATUS_SXL_64;
        /* tp[4] needs to be &cpu_t for M-mode tasks, but we don't know which
         * core will run this task yet. The scheduler will set tp correctly
         * before switching to this task. For now, leave it 0 and trap_exit
         * should NOT restore tp for M-mode tasks. */
    }
#else
    (void)privilege;
    /* v2 compatibility: all tasks run in M-mode */
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

    /* Stack setup (M-mode task uses single stack) */
    task->stack_base = (reg_t *)stack;
    task->stack_size = stack_size;
    task->ustack_base = (reg_t *)stack;   /* Same for M-mode */
    task->ustack_size = stack_size;
    task->kstack_base = NULL;             /* No separate kernel stack */
    task->kstack_top = NULL;

    /* Align stack top to 16 bytes */
    reg_t *stack_top = (reg_t *)((reg_t)stack + stack_size);
    stack_top = (reg_t *)((reg_t)stack_top & ~0xF);

    /* Initialize stack with context (M-mode) */
    task->sp = stack_init(stack_top, func, arg, TASK_PRIV_M);

    /* Add to ready queue */
    sched_add_ready(task);

    critical_exit(state);

    KERNEL_DEBUG("Created task '%s' (id=%d, prio=%d, affinity=0x%x)",
                 name, task->id, priority, task->affinity_mask);

    return task;
}

#if CONFIG_USER_MODE
/**
 * Create a user-mode task with separate kernel and user stacks
 */
tcb_t *task_create_user(const char *name,
                        task_func_t func,
                        void *arg,
                        priority_t priority,
                        uint32_t affinity)
{
    if (g_task_count >= CONFIG_MAX_TASKS) {
        return NULL;
    }

    reg_t state = critical_enter();

    /* Allocate kernel and user stacks */
    int stack_idx = -1;
    for (int i = 0; i < MAX_USER_TASKS; i++) {
        if (!g_user_stack_used[i]) {
            stack_idx = i;
            g_user_stack_used[i] = 1;
            break;
        }
    }
    if (stack_idx < 0) {
        critical_exit(state);
        return NULL;
    }

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
        g_user_stack_used[stack_idx] = 0;
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
    task->held_mutexes = NULL;

    /* User stack setup */
    task->ustack_base = (reg_t *)g_user_ustacks[stack_idx];
    task->ustack_size = USER_USTACK_SIZE;
    task->stack_base = task->ustack_base;  /* Compatibility */
    task->stack_size = task->ustack_size;

    /* Kernel stack setup */
    task->kstack_base = (reg_t *)g_user_kstacks[stack_idx];
    reg_t *kstack_top = (reg_t *)((reg_t)task->kstack_base + USER_KSTACK_SIZE);
    kstack_top = (reg_t *)((reg_t)kstack_top & ~0xF);  /* Align */
    task->kstack_top = kstack_top;

    /* User stack top */
    reg_t *ustack_top = (reg_t *)((reg_t)task->ustack_base + USER_USTACK_SIZE);
    ustack_top = (reg_t *)((reg_t)ustack_top & ~0xF);

    /* Initialize context on KERNEL stack (for trap entry) */
    reg_t *ctx = stack_init(kstack_top, func, arg, TASK_PRIV_U);
    /* Set user SP in context */
    ctx[2] = (reg_t)ustack_top;
    task->sp = ctx;

    /* Add to ready queue */
    sched_add_ready(task);

    critical_exit(state);

    KERNEL_DEBUG("Created user task '%s' (id=%d, prio=%d, ustack=%p, kstack=%p)",
                 name, task->id, priority, task->ustack_base, task->kstack_base);

    return task;
}
#endif /* CONFIG_USER_MODE */

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

void task_delay_internal(tick_t ticks)
{
    /*
     * Internal version: sets up delay but does NOT yield.
     * Used by syscall handler which will call sched_schedule() after.
     */
    if (ticks == 0) {
        return;
    }

    cpu_t *cpu = smp_get_cpu();
    reg_t state = critical_enter();

    tcb_t *task = cpu->current_task;
    tick_t wake_tick = tick_get() + ticks;
    task->wake_tick = wake_tick;
    task->state = TASK_STATE_BLOCKED;
    task->wake_reason = WAKE_REASON_TIMEOUT;

    sched_remove_ready(task);
    delay_add(task, wake_tick);

    critical_exit(state);
}

void task_delay(tick_t ticks)
{
    /*
     * Public API: sets up delay and yields.
     * Used by kernel tasks running in M-mode.
     */
    task_delay_internal(ticks);

    if (ticks > 0) {
        /* Yield to let other tasks run while we wait */
        task_yield();
    }
}

void task_exit(void)
{
    /*
     * Called from syscall handler when user task calls sys_exit().
     * Also callable from kernel tasks.
     *
     * We only mark the task as deleted here. We do NOT call task_yield()
     * because that would execute ecall from within the syscall handler
     * (M-mode), causing nested trap handling issues.
     *
     * The syscall handler will call sched_schedule() after this returns,
     * which will switch to another ready task.
     */
    cpu_t *cpu = smp_get_cpu();
    reg_t state = critical_enter();

    tcb_t *task = cpu->current_task;
    sched_remove_ready(task);
    task->state = TASK_STATE_DELETED;

#if CONFIG_USER_MODE
    /* Free user stack if allocated from pool */
    if (task->kstack_base != NULL) {
        for (int i = 0; i < MAX_USER_TASKS; i++) {
            if ((void *)task->kstack_base == (void *)g_user_kstacks[i]) {
                g_user_stack_used[i] = 0;
                break;
            }
        }
    }
#endif

    critical_exit(state);

    /* Syscall handler will call sched_schedule() to switch task */
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

