/**
 * danieRTOS v2.x - SSD Controller Demo
 *
 * Copyright (c) 2025 Danny Jiang <djiang.tw@gmail.com>
 * SPDX-License-Identifier: MIT
 *
 * Simulates a 4-core SSD controller workload:
 * - Core 0: Front-End (NVMe command processing)
 * - Core 1: Back-End (Flash R/W, FTL operations)
 * - Core 2: GC (Garbage Collection)
 * - Core 3: Misc (Power/Thermal management, SMART)
 *
 * Build: make DEMO=ssd_controller SMP=4
 * Run:   make run
 */

#include "daniertos.h"
#include "smp.h"

/* -----------------------------------------------------------------------------
 * Task Stacks
 * ---------------------------------------------------------------------------*/

static uint8_t stack_frontend[2048] __attribute__((aligned(16)));
static uint8_t stack_backend[2048] __attribute__((aligned(16)));
static uint8_t stack_gc[2048] __attribute__((aligned(16)));
static uint8_t stack_misc[2048] __attribute__((aligned(16)));

/* -----------------------------------------------------------------------------
 * Shared Command Queue (Front-End -> Back-End)
 * ---------------------------------------------------------------------------*/

static queue_t cmd_queue;
static uint8_t cmd_queue_buf[16 * sizeof(uint32_t)];  /* 16 commands */

/* Simulated command counter */
static volatile uint32_t g_cmd_id = 0;

/* -----------------------------------------------------------------------------
 * Front-End Task (Core 0) - NVMe Command Processing
 * ---------------------------------------------------------------------------*/

static void frontend_task_func(void *arg)
{
    (void)arg;
    uart_printf("[%d] [FrontEnd] Started on Core 0 - NVMe command processing\n", tick_get());

    while (1) {
        /* Simulate receiving NVMe command from host */
        uint32_t cmd_id = g_cmd_id++;

        uart_printf("[%d] [FrontEnd] Received NVMe cmd #%d, sending to BackEnd\n",
                    tick_get(), cmd_id);

        /* Send command to Back-End via queue */
        if (!queue_send(&cmd_queue, &cmd_id, 0)) {
            uart_printf("[%d] [FrontEnd] WARNING: Command queue full!\n", tick_get());
        }

        /* Simulate command arrival rate (~100ms between commands) */
        delay_ticks(100);
    }
}

/* -----------------------------------------------------------------------------
 * Back-End Task (Core 1) - Flash R/W, FTL Operations
 * ---------------------------------------------------------------------------*/

static void backend_task_func(void *arg)
{
    (void)arg;
    uart_printf("[%d] [BackEnd] Started on Core 1 - Flash R/W, FTL ops\n", tick_get());

    while (1) {
        uint32_t cmd_id;

        /* Wait for command from Front-End (wait up to 1 second) */
        if (queue_receive(&cmd_queue, &cmd_id, 1000)) {
            uart_printf("[%d] [BackEnd] Processing cmd #%d (Flash R/W)\n",
                        tick_get(), cmd_id);

            /* Simulate Flash operation latency (~50ms) */
            delay_ticks(50);

            uart_printf("[%d] [BackEnd] Completed cmd #%d\n",
                        tick_get(), cmd_id);
        }
    }
}

/* -----------------------------------------------------------------------------
 * GC Task (Core 2) - Garbage Collection
 * ---------------------------------------------------------------------------*/

static void gc_task_func(void *arg)
{
    (void)arg;
    uart_printf("[%d] [GC] Started on Core 2 - Garbage Collection\n", tick_get());
    uint32_t gc_cycle = 0;

    while (1) {
        /* GC runs periodically in background */
        uart_printf("[%d] [GC] Starting GC cycle #%d\n",
                    tick_get(), gc_cycle);

        /* Simulate GC work (~200ms) */
        delay_ticks(200);

        uart_printf("[%d] [GC] Completed GC cycle #%d, reclaimed blocks\n",
                    tick_get(), gc_cycle++);

        /* Wait before next GC cycle (~500ms) */
        delay_ticks(500);
    }
}

/* -----------------------------------------------------------------------------
 * Misc Task (Core 3) - Power/Thermal Management, SMART
 * ---------------------------------------------------------------------------*/

static void misc_task_func(void *arg)
{
    (void)arg;
    uart_printf("[%d] [Misc] Started on Core 3 - Power/Thermal/SMART\n", tick_get());
    uint32_t report_id = 0;

    while (1) {
        /* Simulate temperature monitoring */
        uint32_t temp = 45 + (tick_get() % 10);  /* 45-54deg C */

        uart_printf("[%d] [Misc] Health report #%d - Temp=%dC, Power=OK\n",
                    tick_get(), report_id++, temp);

        /* Report every 1 second */
        delay_ticks(1000);
    }
}

/* -----------------------------------------------------------------------------
 * Demo Initialization
 * ---------------------------------------------------------------------------*/

void demo_init(void)
{
    uart_puts("\n");
    uart_puts("+==============================================================+\n");
    uart_puts("|           SSD Controller Demo (4-Core SMP)                   |\n");
    uart_puts("+==============================================================+\n");
    uart_puts("|  Core 0: Front-End  - NVMe command processing                |\n");
    uart_puts("|  Core 1: Back-End   - Flash R/W, FTL operations              |\n");
    uart_puts("|  Core 2: GC         - Garbage Collection                     |\n");
    uart_puts("|  Core 3: Misc       - Power/Thermal/SMART                    |\n");
    uart_puts("+==============================================================+\n\n");

    uart_printf("CONFIG_NUM_CORES = %d\n\n", CONFIG_NUM_CORES);

    /* Verify we have at least 4 cores */
    if (CONFIG_NUM_CORES < 4) {
        uart_puts("ERROR: SSD Controller demo requires SMP=4 (4 cores)\n");
        uart_puts("Please rebuild with: make clean && make DEMO=ssd_controller SMP=4\n");
        while (1) { asm volatile("wfi"); }
    }

    /* Initialize command queue (16 commands of uint32_t) */
    queue_init(&cmd_queue, cmd_queue_buf, sizeof(uint32_t), 16);
    uart_puts("Command queue initialized (16 x uint32_t)\n");

    /* Create Front-End task (pinned to Core 0) */
    task_create("FrontEnd",
                frontend_task_func,
                NULL,
                3,  /* High priority */
                stack_frontend,
                sizeof(stack_frontend),
                (1U << 0));  /* Core 0 only */

    /* Create Back-End task (pinned to Core 1) */
    task_create("BackEnd",
                backend_task_func,
                NULL,
                3,  /* High priority */
                stack_backend,
                sizeof(stack_backend),
                (1U << 1));  /* Core 1 only */

    /* Create GC task (pinned to Core 2) */
    task_create("GC",
                gc_task_func,
                NULL,
                1,  /* Low priority (background) */
                stack_gc,
                sizeof(stack_gc),
                (1U << 2));  /* Core 2 only */

    /* Create Misc task (pinned to Core 3) */
    task_create("Misc",
                misc_task_func,
                NULL,
                1,  /* Low priority (background) */
                stack_misc,
                sizeof(stack_misc),
                (1U << 3));  /* Core 3 only */

    uart_puts("\nAll tasks created. Starting scheduler...\n\n");
}

