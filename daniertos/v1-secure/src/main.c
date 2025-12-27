/**
 * danieRTOS - Main Entry Point
 *
 * Copyright (c) 2025 Danny Jiang <djiang.tw@gmail.com>
 * SPDX-License-Identifier: MIT
 *
 * Demo is selected at build time via DEMO=xxx
 * See src/demos/ for available demos.
 */

#include "daniertos.h"

/* Demo init function - implemented in selected demo file */
extern void demo_init(void);

/* -----------------------------------------------------------------------------
 * Kernel Initialization
 * ---------------------------------------------------------------------------*/

void kernel_init(void)
{
    uart_init();

    uart_puts("\n");
    uart_puts("========================================\n");
    uart_puts("  danieRTOS v0.1 - RISC-V 64-bit\n");
    uart_puts("========================================\n\n");

    sched_init();
    task_init();
    tick_init();
}

void kernel_start(void)
{
    sched_start();
    /* Never returns */
}

/* -----------------------------------------------------------------------------
 * Main Entry Point
 * ---------------------------------------------------------------------------*/

int main(void)
{
    kernel_init();

    /* Initialize and create tasks for selected demo */
    demo_init();

    kernel_start();

    return 0;
}

