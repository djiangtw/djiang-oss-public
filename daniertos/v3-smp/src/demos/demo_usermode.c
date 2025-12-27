/**
 * danieRTOS v3.x - User Mode Demo (SMP + User Mode)
 *
 * Copyright (c) 2025 Danny Jiang <djiang.tw@gmail.com>
 * SPDX-License-Identifier: MIT
 *
 * This demo demonstrates:
 *   1. User-mode tasks running in U-mode privilege
 *   2. System calls for I/O (sys_puts, sys_putchar)
 *   3. System calls for task management (sys_yield, sys_delay)
 *   4. Multiple user tasks running on multiple cores
 */

#include "daniertos.h"
#include "smp.h"
#include "user_syscall.h"

/* -----------------------------------------------------------------------------
 * Demo Configuration
 * ---------------------------------------------------------------------------*/

#define NUM_USER_TASKS  3

/* -----------------------------------------------------------------------------
 * User Task Functions
 *
 * These functions will run in U-mode and can only use syscalls for I/O.
 * ---------------------------------------------------------------------------*/

static void __attribute__((unused)) user_task_a(void *arg)
{
    (void)arg;
    int count = 0;

    sys_puts("[UserTaskA] Started in U-mode!\n");

    while (count < 10) {
        sys_putchar('A');
        sys_putchar('0' + (count % 10));
        sys_putchar(' ');
        sys_delay(50);  /* Delay 50 ticks via syscall */
        count++;
    }

    sys_puts("\n[UserTaskA] Done!\n");
    sys_exit();  /* Clean exit */
}

static void __attribute__((unused)) user_task_b(void *arg)
{
    (void)arg;
    int count = 0;

    sys_puts("[UserTaskB] Started in U-mode!\n");

    while (count < 10) {
        sys_putchar('B');
        sys_putchar('0' + (count % 10));
        sys_putchar(' ');
        sys_delay(70);  /* Slightly different timing */
        count++;
    }

    sys_puts("\n[UserTaskB] Done!\n");
    sys_exit();
}

static void __attribute__((unused)) user_task_migrate(void *arg)
{
    (void)arg;
    int count = 0;

    sys_puts("[MigrateTask] Started, can run on any core!\n");

    while (count < 8) {
        sys_printf("[MigrateTask] Tick=%d, iteration=%d\n",
                   (int)sys_get_tick(), count);
        sys_yield();  /* Give other tasks a chance */
        sys_delay(80);
        count++;
    }

    sys_puts("[MigrateTask] Done!\n");
    sys_exit();
}

/* -----------------------------------------------------------------------------
 * Kernel Monitor Task (M-mode)
 *
 * This task runs in M-mode to monitor the system.
 * ---------------------------------------------------------------------------*/

static uint8_t monitor_stack[8192] __attribute__((aligned(16)));

static void monitor_task(void *arg)
{
    (void)arg;

    uart_puts("[Monitor] Kernel monitor started in M-mode\n");

    int count = 0;
    while (count < 5) {
        uart_printf("[Monitor] Loop %d, tick=%d\n", count++, (int)tick_get());
        task_delay(100);  /* Sleep for 100 ticks */
    }
    uart_puts("[Monitor] Done!\n");
}

/* -----------------------------------------------------------------------------
 * Demo Entry Point
 * ---------------------------------------------------------------------------*/

void demo_init(void)
{
    uart_puts("\n=== danieRTOS v3.x User Mode Demo ===\n");
    uart_printf("Number of cores: %d\n", CONFIG_NUM_CORES);

    /* Create M-mode kernel monitor task, pinned to core 1 */
    uart_puts("Creating Monitor task (M-mode, Core 1)...\n");
    task_create("Monitor", monitor_task, NULL, 3, monitor_stack, sizeof(monitor_stack), (1U << 1));

    /* Create U-mode user tasks */
    uart_puts("Creating User tasks (U-mode)...\n");
    task_create_user("UserA", user_task_a, NULL, 2, CONFIG_CORE_ANY);
    task_create_user("UserB", user_task_b, NULL, 2, CONFIG_CORE_ANY);

    uart_puts("All tasks created. Starting scheduler...\n");
}

