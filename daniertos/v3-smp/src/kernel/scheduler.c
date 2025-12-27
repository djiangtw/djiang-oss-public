/**
 * danieRTOS v3.x - Scheduler Implementation (SMP + User Mode)
 *
 * Copyright (c) 2025 Danny Jiang <djiang.tw@gmail.com>
 * SPDX-License-Identifier: MIT
 *
 * Priority-based preemptive scheduler with round-robin within same priority.
 * Uses spinlock to protect shared ready queue across cores.
 *
 * v3.x additions:
 *   - Update cpu->kernel_trap_sp on context switch for U-mode tasks
 *   - Call pmp_update_task() for dynamic PMP configuration
 */

#include "daniertos.h"
#include "smp.h"
#include "spinlock.h"
#include "pmp.h"

/* -----------------------------------------------------------------------------
 * State
 * ---------------------------------------------------------------------------*/

/* Global ready queue - one per priority level (protected by g_sched_lock) */
static tcb_t *g_ready_queue[CONFIG_MAX_PRIORITY];

/* Global scheduler lock (defined in main.c) */
extern spinlock_t g_sched_lock;

/* Scheduler state */
static volatile sched_state_t g_sched_state = SCHED_STATE_STOPPED;

/* External declarations from task.c */
extern tcb_t *task_get_idle_for_core(uint64_t hartid);

/* -----------------------------------------------------------------------------
 * Ready Queue Management
 * ---------------------------------------------------------------------------*/

void sched_add_ready(tcb_t *task)
{
    if (task == NULL || task->state == TASK_STATE_DELETED) {
        return;
    }

    /* Sanity check: task pointer should be in valid range */
    if ((reg_t)task < 0x80000000 || (reg_t)task > 0x90000000) {
        uart_printf("[FATAL] sched_add_ready: bad task=%p\n", task);
        while(1) { asm volatile("wfi"); }
    }

    priority_t prio = task->priority;
    if (prio >= CONFIG_MAX_PRIORITY) {
        prio = CONFIG_MAX_PRIORITY - 1;
    }

    /* Add to end of queue for round-robin */
    tcb_t **pp = &g_ready_queue[prio];
    while (*pp != NULL) {
        /* Sanity check linked list */
        if ((reg_t)*pp < 0x80000000 || (reg_t)*pp > 0x90000000) {
            uart_printf("[FATAL] sched_add_ready: corrupted queue at prio=%d, pp=%p\n", prio, *pp);
            while(1) { asm volatile("wfi"); }
        }
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
 * Scheduler Core (SMP-Safe)
 * ---------------------------------------------------------------------------*/

/**
 * Select highest priority ready task for a specific core
 * NOTE: Caller must hold g_sched_lock
 */
static tcb_t *select_next_task_for_core(uint64_t hartid)
{
    uint32_t core_mask = (1U << hartid);

    /* Search from highest to lowest priority */
    for (int prio = CONFIG_MAX_PRIORITY - 1; prio >= 0; prio--) {
        tcb_t *task = g_ready_queue[prio];
        while (task != NULL) {
            /* Sanity check: task pointer should be in valid range */
            if ((reg_t)task < 0x80000000 || (reg_t)task > 0x90000000) {
                uart_printf("[FATAL] select_next_task: corrupted task=%p in prio=%d\n", task, prio);
                while(1) { asm volatile("wfi"); }
            }
            /* Check if task can run on this core (affinity check) */
            if (task->affinity_mask & core_mask) {
                return task;
            }
            task = task->next;
        }
    }

    /* No ready task - return idle task for this core */
    return task_get_idle_for_core(hartid);
}

void sched_init(void)
{
    for (int i = 0; i < CONFIG_MAX_PRIORITY; i++) {
        g_ready_queue[i] = NULL;
    }
    g_sched_state = SCHED_STATE_STOPPED;
    spinlock_init(&g_sched_lock);
}

void sched_start(void)
{
    cpu_t *cpu = smp_get_cpu();

    /* Select first task with lock held */
    reg_t mstatus = spinlock_lock_irqsave(&g_sched_lock);
    tcb_t *task = select_next_task_for_core(cpu->hartid);
    KERNEL_ASSERT(task != NULL);

    sched_remove_ready(task);
    task->state = TASK_STATE_RUNNING;
    cpu->current_task = task;
    spinlock_unlock_irqrestore(&g_sched_lock, mstatus);

    g_sched_state = SCHED_STATE_RUNNING;

    uart_printf("[Core %d] Starting scheduler, first task: %s\n",
                cpu->hartid, task->name);

#if CONFIG_USER_MODE
    /* v3.x: Set up kernel_trap_sp for U-mode tasks */
    if (task->kstack_top != NULL) {
        cpu->kernel_trap_sp = (uint64_t)task->kstack_top;
    }

    /*
     * v3.x: Set mscratch to &cpu_t before entering U-mode
     *
     * When trap_exit returns to U-mode (via mret), any subsequent trap
     * will need to find cpu_t. The trap_entry code does:
     *   csrrw tp, mscratch, tp   // Swap tp and mscratch
     * After this, tp = &cpu_t (from mscratch), and mscratch = old tp.
     */
    csr_write(mscratch, (reg_t)cpu);
#endif

    /* Initialize timer for BSP */
    timer_init();

    /* Debug: print task's initial mstatus */
    /* Debug disabled: uart_printf("[DEBUG] task->sp[33] (mstatus) = %lx\n", task->sp[33]); */

    /* Load context and jump to first task */
    asm volatile (
        "mv     sp, %0      \n"
        "j      trap_exit   \n"
        :
        : "r"(task->sp)
    );

    __builtin_unreachable();
}

void sched_start_ap(void)
{
    /* Disable interrupts first to avoid races */
    csr_clear(mstatus, MSTATUS_MIE);

    cpu_t *cpu = smp_get_cpu();

    /* Select first task for this AP */
    reg_t mstatus = spinlock_lock_irqsave(&g_sched_lock);

    tcb_t *task = select_next_task_for_core(cpu->hartid);
    if (task == NULL) {
        spinlock_unlock_irqrestore(&g_sched_lock, mstatus);
        /* No task for this core, just idle */
        while(1) { asm volatile("wfi"); }
    }

    sched_remove_ready(task);
    task->state = TASK_STATE_RUNNING;
    cpu->current_task = task;
    spinlock_unlock_irqrestore(&g_sched_lock, mstatus);

    uart_printf("[Core %d] Starting scheduler, first task: %s\n",
                cpu->hartid, task->name);

#if CONFIG_USER_MODE
    /* v3.x: Set up kernel_trap_sp for U-mode tasks */
    if (task->kstack_top != NULL) {
        cpu->kernel_trap_sp = (uint64_t)task->kstack_top;
    }
    /* Set mscratch to &cpu_t */
    csr_write(mscratch, (reg_t)cpu);
#endif

    /* Initialize timer for this AP */
    timer_init_ap(cpu->hartid);
    asm volatile (
        "mv     sp, %0      \n"
        "j      trap_exit   \n"
        :
        : "r"(task->sp)
    );

    __builtin_unreachable();
}

sched_state_t sched_get_state(void)
{
    return g_sched_state;
}

bool sched_in_isr(void)
{
    cpu_t *cpu = smp_get_cpu();
    return cpu->irq_nesting > 0;
}

void sched_request_switch(void)
{
    cpu_t *cpu = smp_get_cpu();
    cpu->need_reschedule = 1;
}

/**
 * Main scheduler function (SMP-Safe)
 * Called with interrupts disabled
 */
reg_t *sched_schedule(reg_t *current_sp)
{
    cpu_t *cpu = smp_get_cpu();
    tcb_t *current = cpu->current_task;
    tcb_t *next;

    /* Acquire scheduler lock */
    spinlock_acquire(&g_sched_lock);

    /* Save current SP */
    if (current != NULL) {
        current->sp = current_sp;

        /* Move current to end of its priority queue (round-robin) */
        if (current->state == TASK_STATE_RUNNING) {
            current->state = TASK_STATE_READY;
            sched_add_ready(current);
        }
    }

    /* Select next task for this core */
    next = select_next_task_for_core(cpu->hartid);

    /* Remove from ready queue and mark as running */
    sched_remove_ready(next);
    next->state = TASK_STATE_RUNNING;
    cpu->current_task = next;
    cpu->need_reschedule = 0;

#if CONFIG_USER_MODE
    /*
     * v3.x: Update cpu->kernel_trap_sp for U-mode tasks
     *
     * When mret returns to U-mode and a trap occurs, trap_entry needs to know
     * where the kernel stack is. We store this in cpu->kernel_trap_sp.
     *
     * For U-mode tasks: use task->kstack_top
     * For M-mode tasks: use the current kernel stack (no separate kstack)
     */
    if (next->kstack_top != NULL) {
        cpu->kernel_trap_sp = (uint64_t)next->kstack_top;
    }
    /* Note: For M-mode tasks, kernel_trap_sp stays unchanged (uses main stack) */

    /* Update PMP for task's user stack (v3p2: per-task PMP configuration) */
    pmp_update_task(next);
#endif

    spinlock_release(&g_sched_lock);

    return next->sp;
}

reg_t *sched_yield(reg_t *current_sp)
{
    return sched_schedule(current_sp);
}

/* -----------------------------------------------------------------------------
 * ISR Nesting (Per-Core)
 * ---------------------------------------------------------------------------*/

void sched_enter_isr(void)
{
    cpu_t *cpu = smp_get_cpu();
    cpu->irq_nesting++;
}

void sched_exit_isr(void)
{
    cpu_t *cpu = smp_get_cpu();
    if (cpu->irq_nesting > 0) {
        cpu->irq_nesting--;
    }
}

bool sched_switch_pending(void)
{
    cpu_t *cpu = smp_get_cpu();
    return cpu->need_reschedule != 0;
}
