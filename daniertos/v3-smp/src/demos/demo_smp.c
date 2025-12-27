/**
 * danieRTOS v2.x - SMP Demo
 *
 * Copyright (c) 2025 Danny Jiang <djiang.tw@gmail.com>
 * SPDX-License-Identifier: MIT
 *
 * Demonstrates multi-core task execution:
 * - Tasks running on Core 0
 * - Tasks running on Core 1
 * - Tasks that can migrate between cores
 */

#include "daniertos.h"
#include "smp.h"

/* -----------------------------------------------------------------------------
 * Task Stacks
 * ---------------------------------------------------------------------------*/

static uint8_t stack_core0_task[2048] __attribute__((aligned(16)));
static uint8_t stack_core1_task[2048] __attribute__((aligned(16)));
static uint8_t stack_migrating_task[2048] __attribute__((aligned(16)));

/* -----------------------------------------------------------------------------
 * Core 0 Only Task
 * ---------------------------------------------------------------------------*/

static void core0_task_func(void *arg)
{
    (void)arg;
    uart_puts(">>> Core0Task started!\n");
    int count = 0;

    while (1) {
        cpu_t *cpu = smp_get_cpu();
        uart_printf("[Tick %5d] Core0Task on hart %d, count=%d\n",
                    tick_get(), cpu->hartid, count++);
        delay_ticks(500);
    }
}

/* -----------------------------------------------------------------------------
 * Core 1 Only Task
 * ---------------------------------------------------------------------------*/

static void core1_task_func(void *arg)
{
    (void)arg;
    uart_puts(">>> Core1Task started!\n");
    int count = 0;

    while (1) {
        cpu_t *cpu = smp_get_cpu();
        uart_printf("[Tick %5d] Core1Task on hart %d, count=%d\n",
                    tick_get(), cpu->hartid, count++);
        delay_ticks(700);
    }
}

/* -----------------------------------------------------------------------------
 * Migrating Task (can run on any core)
 * ---------------------------------------------------------------------------*/

static void migrating_task_func(void *arg)
{
    (void)arg;
    int count = 0;
    
    while (1) {
        cpu_t *cpu = smp_get_cpu();
        uart_printf("[Tick %5d] MigratingTask on hart %d, count=%d\n",
                    tick_get(), cpu->hartid, count++);
        delay_ticks(300);
    }
}

/* -----------------------------------------------------------------------------
 * Demo Initialization
 * ---------------------------------------------------------------------------*/

void demo_init(void)
{
    uart_puts("\n=== SMP Demo ===\n");
    uart_printf("Number of cores: %d\n", CONFIG_NUM_CORES);
    uart_puts("Creating tasks:\n");
    uart_puts("  - Core0Task: runs only on Core 0\n");
    uart_puts("  - Core1Task: runs only on Core 1\n");
    uart_puts("  - MigratingTask: can migrate between cores\n\n");

    /* Create task pinned to Core 0 */
    task_create("Core0Task",
                core0_task_func,
                NULL,
                2,  /* priority */
                stack_core0_task,
                sizeof(stack_core0_task),
                (1U << 0));  /* Core 0 only */

    /* Create task pinned to Core 1 */
    task_create("Core1Task",
                core1_task_func,
                NULL,
                2,  /* priority */
                stack_core1_task,
                sizeof(stack_core1_task),
                (1U << 1));  /* Core 1 only */

    /* Create task that can migrate */
    task_create("MigratingTask",
                migrating_task_func,
                NULL,
                1,  /* lower priority */
                stack_migrating_task,
                sizeof(stack_migrating_task),
                CONFIG_CORE_ANY);  /* Any core */
}

