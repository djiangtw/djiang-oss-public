/**
 * danieRTOS v3.x - 8-Core SMP Demo
 *
 * Copyright (c) 2025 Danny Jiang <djiang.tw@gmail.com>
 * SPDX-License-Identifier: MIT
 *
 * Demonstrates all 8 cores running tasks simultaneously.
 * Each core has a dedicated task pinned to it.
 */

#include "daniertos.h"
#include "smp.h"

/* -----------------------------------------------------------------------------
 * Task Stacks (one per core)
 * ---------------------------------------------------------------------------*/

static uint8_t stack_core0[2048] __attribute__((aligned(16)));
static uint8_t stack_core1[2048] __attribute__((aligned(16)));
static uint8_t stack_core2[2048] __attribute__((aligned(16)));
static uint8_t stack_core3[2048] __attribute__((aligned(16)));
static uint8_t stack_core4[2048] __attribute__((aligned(16)));
static uint8_t stack_core5[2048] __attribute__((aligned(16)));
static uint8_t stack_core6[2048] __attribute__((aligned(16)));
static uint8_t stack_core7[2048] __attribute__((aligned(16)));

static uint8_t *stacks[8] = {
    stack_core0, stack_core1, stack_core2, stack_core3,
    stack_core4, stack_core5, stack_core6, stack_core7
};

/* -----------------------------------------------------------------------------
 * Per-Core Task
 * ---------------------------------------------------------------------------*/

static void core_task_func(void *arg)
{
    int core_id = (int)(uintptr_t)arg;
    int count = 0;

    uart_printf(">>> Core%dTask started!\n", core_id);

    while (1) {
        cpu_t *cpu = smp_get_cpu();
        uart_printf("[Tick %5d] Core%dTask on hart %d, count=%d\n",
                    tick_get(), core_id, cpu->hartid, count++);
        /* Stagger delays so output doesn't overlap too much */
        delay_ticks(500 + core_id * 50);
    }
}

/* -----------------------------------------------------------------------------
 * Demo Initialization
 * ---------------------------------------------------------------------------*/

void demo_init(void)
{
    uart_puts("\n=== 8-Core SMP Demo ===\n");
    uart_printf("Number of cores: %d\n", CONFIG_NUM_CORES);
    uart_puts("Creating one task per core (pinned):\n\n");

    int num_cores = CONFIG_NUM_CORES;
    if (num_cores > 8) num_cores = 8;

    for (int i = 0; i < num_cores; i++) {
        char name[16];
        /* Simple name construction without snprintf */
        name[0] = 'C'; name[1] = 'o'; name[2] = 'r'; name[3] = 'e';
        name[4] = '0' + i;
        name[5] = 'T'; name[6] = 'a'; name[7] = 's'; name[8] = 'k';
        name[9] = '\0';

        uart_printf("  - %s: pinned to Core %d\n", name, i);

        task_create(name,
                    core_task_func,
                    (void *)(uintptr_t)i,
                    2,  /* priority */
                    stacks[i],
                    2048,
                    (1U << i));  /* Pin to core i */
    }

    uart_puts("\n");
}

