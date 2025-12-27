/**
 * danieRTOS - Basic Demo
 *
 * Copyright (c) 2025 Danny Jiang <djiang.tw@gmail.com>
 * SPDX-License-Identifier: MIT
 *
 * Simple demo: 3 tasks run for a few iterations then exit.
 * Demonstrates basic task switching and delays.
 */

#include "daniertos.h"

/* Task stacks */
static uint8_t stack1[CONFIG_MINIMAL_STACK_SIZE] __attribute__((aligned(16)));
static uint8_t stack2[CONFIG_MINIMAL_STACK_SIZE] __attribute__((aligned(16)));
static uint8_t stack3[CONFIG_MINIMAL_STACK_SIZE] __attribute__((aligned(16)));

static volatile int tasks_done = 0;

static void task_a(void *arg)
{
    (void)arg;
    for (int i = 0; i < 5; i++) {
        log_printf("Task A: %d/5\n", i + 1);
        delay_ms(500);
    }
    log_printf("Task A: done\n");
    tasks_done++;
    while (1) delay_ms(1000);  /* Wait for others */
}

static void task_b(void *arg)
{
    (void)arg;
    for (int i = 0; i < 4; i++) {
        log_printf("Task B: %d/4\n", i + 1);
        delay_ms(700);
    }
    log_printf("Task B: done\n");
    tasks_done++;
    while (1) delay_ms(1000);
}

static void task_c(void *arg)
{
    (void)arg;
    for (int i = 0; i < 3; i++) {
        log_printf("Task C: %d/3\n", i + 1);
        delay_ms(1000);
    }
    log_printf("Task C: done\n");
    tasks_done++;

    /* Wait for all tasks to complete */
    while (tasks_done < 3) {
        delay_ms(100);
    }

    uart_printf("\n========================================\n");
    uart_printf("  All tasks completed - Demo End!\n");
    uart_printf("========================================\n");

    /* Halt */
    while (1) {
        asm volatile ("wfi");
    }
}

void demo_init(void)
{
    task_create("task_a", task_a, NULL, 1, stack1, sizeof(stack1));
    task_create("task_b", task_b, NULL, 2, stack2, sizeof(stack2));
    task_create("task_c", task_c, NULL, 3, stack3, sizeof(stack3));

    uart_printf("Basic Demo - 3 tasks\n\n");
}

