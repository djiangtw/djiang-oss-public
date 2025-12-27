/**
 * danieRTOS v1.x - Basic Demo
 *
 * Copyright (c) 2025 Danny Jiang <djiang.tw@gmail.com>
 * SPDX-License-Identifier: MIT
 *
 * Simple demo: 3 tasks run for a few iterations then exit.
 * Demonstrates basic task switching and delays using syscalls.
 *
 * v1.x: User tasks run in U-mode and must use syscalls to interact
 * with the kernel. Direct kernel function calls are not allowed.
 */

#include "daniertos.h"
#include "user_syscall.h"  /* User-mode syscall API */

/* Task stacks */
static uint8_t stack1[CONFIG_MINIMAL_STACK_SIZE] __attribute__((aligned(16)));
static uint8_t stack2[CONFIG_MINIMAL_STACK_SIZE] __attribute__((aligned(16)));
static uint8_t stack3[CONFIG_MINIMAL_STACK_SIZE] __attribute__((aligned(16)));

static volatile int tasks_done = 0;

/* -----------------------------------------------------------------------------
 * User Task Functions
 *
 * These run in U-mode and MUST use sys_* functions (syscalls) to interact
 * with the kernel. Direct calls to kernel functions like uart_printf(),
 * delay_ms(), etc. will cause Instruction Access Faults.
 * ---------------------------------------------------------------------------*/

static void task_a(void *arg)
{
    (void)arg;
    for (int i = 0; i < 5; i++) {
        sys_log("Task A: %d/5\n", i + 1);
        sys_delay_ms(500);
    }
    sys_log("Task A: done\n");
    tasks_done++;
    while (1) sys_delay_ms(1000);  /* Wait for others */
}

static void task_b(void *arg)
{
    (void)arg;
    for (int i = 0; i < 4; i++) {
        sys_log("Task B: %d/4\n", i + 1);
        sys_delay_ms(700);
    }
    sys_log("Task B: done\n");
    tasks_done++;
    while (1) sys_delay_ms(1000);
}

static void task_c(void *arg)
{
    (void)arg;
    for (int i = 0; i < 3; i++) {
        sys_log("Task C: %d/3\n", i + 1);
        sys_delay_ms(1000);
    }
    sys_log("Task C: done\n");
    tasks_done++;

    /* Wait for all tasks to complete */
    while (tasks_done < 3) {
        sys_delay_ms(100);
    }

    sys_puts("\n========================================\n");
    sys_puts("  All tasks completed - Demo End!\n");
    sys_puts("========================================\n");

    /* Exit task properly instead of busy loop with wfi (which needs M-mode) */
    sys_exit();
}

/* -----------------------------------------------------------------------------
 * Demo Initialization (runs in M-mode from main.c)
 * ---------------------------------------------------------------------------*/

void demo_init(void)
{
    task_create("task_a", task_a, NULL, 1, stack1, sizeof(stack1));
    task_create("task_b", task_b, NULL, 2, stack2, sizeof(stack2));
    task_create("task_c", task_c, NULL, 3, stack3, sizeof(stack3));

    uart_printf("Basic Demo - 3 tasks (U-mode)\n\n");
}

