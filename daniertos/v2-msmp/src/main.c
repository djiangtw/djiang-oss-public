/**
 * danieRTOS v2.x - Main Entry Point (SMP Edition)
 *
 * Copyright (c) 2025 Danny Jiang <djiang.tw@gmail.com>
 * SPDX-License-Identifier: MIT
 *
 * Multi-core entry:
 * - main() is called by BSP (Core 0)
 * - main_secondary() is called by AP (Core 1+)
 */

#include "daniertos.h"
#include "smp.h"
#include "spinlock.h"

/* Demo init function - implemented in selected demo file */
extern void demo_init(void);

/* Spinlock for scheduler */
spinlock_t g_sched_lock = SPINLOCK_INIT;

/* Boot flag - defined in startup.S */
extern volatile uint32_t g_boot_flag;

/* -----------------------------------------------------------------------------
 * BSP (Core 0) Initialization
 * ---------------------------------------------------------------------------*/

void kernel_init(void)
{
    uart_init();

    uart_puts("\n");
    uart_puts("========================================\n");
    uart_puts("  danieRTOS v2.0 - RISC-V 64-bit SMP\n");
    uart_puts("========================================\n\n");

    uart_printf("[Core 0] Initializing kernel...\n");

    /* Initialize SMP for BSP */
    smp_init_bsp();

    /* Initialize scheduler and tasks */
    sched_init();
    task_init();
    tick_init();

    uart_printf("[Core 0] Kernel initialized\n");
}

void kernel_start(void)
{
    /* Signal APs that kernel is ready */
    uart_printf("[Core 0] Signaling APs to start\n");
    __sync_synchronize();  /* Full memory barrier */
    g_boot_flag = 1;
    __sync_synchronize();

    uart_printf("[Core 0] Starting scheduler\n");
    sched_start();
    /* Never returns */
}

/* -----------------------------------------------------------------------------
 * Main Entry Point (BSP - Core 0)
 * ---------------------------------------------------------------------------*/

int main(void)
{
    kernel_init();

    /* Initialize and create tasks for selected demo */
    demo_init();

    kernel_start();

    return 0;
}

/* -----------------------------------------------------------------------------
 * Secondary Entry Point (AP - Core 1+)
 * ---------------------------------------------------------------------------*/

void main_secondary(uint64_t hartid)
{
    /* Initialize SMP for this AP */
    smp_init_ap(hartid);

    /* Initialize per-core timer */
    timer_init_ap(hartid);

    uart_printf("[Core %d] Starting scheduler\n", hartid);

    /* Start scheduler on this core */
    sched_start_ap();

    /* Never returns */
    while (1) {
        asm volatile("wfi");
    }
}

