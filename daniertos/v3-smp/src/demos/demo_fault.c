/**
 * danieRTOS v3p2 - Fault Isolation Demo
 *
 * Copyright (c) 2025 Danny Jiang <djiang.tw@gmail.com>
 * SPDX-License-Identifier: MIT
 *
 * This demo demonstrates fault isolation:
 *   1. A "bad" user task that crashes (access violation)
 *   2. A "good" user task that continues running
 *   3. System remains stable after the bad task is terminated
 */

#include "daniertos.h"
#include "smp.h"
#include "user_syscall.h"

/* -----------------------------------------------------------------------------
 * User Task: Bad Task (will crash)
 *
 * Note: USER_FUNC places this in .user_text section for PMP access
 * ---------------------------------------------------------------------------*/

USER_FUNC static void bad_task(void *arg)
{
    (void)arg;

    sys_puts("[BadTask] Started in U-mode\n");
    sys_puts("[BadTask] Counting down to crash...\n");

    for (int i = 3; i > 0; i--) {
        sys_printf("[BadTask] %d...\n", i);
        sys_delay(100);
    }

    sys_puts("[BadTask] Attempting to access kernel memory...\n");

    /* Try to access kernel memory (0x80000000) - should cause access fault */
    volatile int *kernel_ptr = (int *)0x80000000;
    *kernel_ptr = 42;  /* This will trigger an access fault! */

    /* Should never reach here */
    sys_puts("[BadTask] ERROR: This should not print!\n");
    sys_exit();
}

/* -----------------------------------------------------------------------------
 * User Task: Good Task (keeps running)
 * ---------------------------------------------------------------------------*/

USER_FUNC static void good_task(void *arg)
{
    (void)arg;
    int count = 0;

    sys_puts("[GoodTask] Started in U-mode\n");

    while (count < 15) {
        sys_printf("[GoodTask] Still running... %d\n", count);
        sys_delay(150);
        count++;
    }

    sys_puts("[GoodTask] Done! (System remained stable)\n");
    sys_exit();
}

/* -----------------------------------------------------------------------------
 * User Task: Another Good Task
 * ---------------------------------------------------------------------------*/

USER_FUNC static void another_task(void *arg)
{
    (void)arg;

    sys_puts("[AnotherTask] Started\n");

    for (int i = 0; i < 10; i++) {
        sys_printf("[AnotherTask] Working... %d\n", i);
        sys_delay(200);
    }

    sys_puts("[AnotherTask] Done!\n");
    sys_exit();
}

/* -----------------------------------------------------------------------------
 * Kernel Monitor Task (M-mode)
 * ---------------------------------------------------------------------------*/

static uint8_t monitor_stack[8192] __attribute__((aligned(16)));

static void monitor_task(void *arg)
{
    (void)arg;
    uart_puts("[Monitor] Kernel monitor started\n");

    for (int i = 0; i < 12; i++) {
        uart_printf("[Monitor] tick=%d, system OK\n", (int)tick_get());
        task_delay(250);
    }
    uart_puts("[Monitor] All done - fault isolation verified!\n");
}

/* -----------------------------------------------------------------------------
 * Demo Entry Point
 * ---------------------------------------------------------------------------*/

void demo_init(void)
{
    uart_puts("\n=== danieRTOS v3p2 Fault Isolation Demo ===\n");
    uart_puts("BadTask will crash, but GoodTask should keep running.\n\n");

    /* Create M-mode monitor */
    task_create("Monitor", monitor_task, NULL, 4,
                monitor_stack, sizeof(monitor_stack), (1U << 1));

    /* Create U-mode user tasks */
    task_create_user("BadTask", bad_task, NULL, 2, CONFIG_CORE_ANY);
    task_create_user("GoodTask", good_task, NULL, 2, CONFIG_CORE_ANY);
    task_create_user("AnotherTask", another_task, NULL, 2, CONFIG_CORE_ANY);

    uart_puts("All tasks created. Starting scheduler...\n\n");
}

