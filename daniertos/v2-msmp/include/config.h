/**
 * danieRTOS v2.x - Configuration (SMP Edition)
 *
 * Copyright (c) 2025 Danny Jiang <djiang.tw@gmail.com>
 * SPDX-License-Identifier: MIT
 */

#ifndef DANIERTOS_CONFIG_H
#define DANIERTOS_CONFIG_H

/* -----------------------------------------------------------------------------
 * SMP Configuration
 * ---------------------------------------------------------------------------*/

/**
 * Number of CPU cores
 * Can be overridden via Makefile: make SMP=4
 * Supported values: 2, 3, 4, 6, 8
 */
#ifndef CONFIG_NUM_CORES
#define CONFIG_NUM_CORES            2
#endif

/** Maximum supported cores (for static allocation) */
#define CONFIG_MAX_CORES            8

/** Core affinity: allow task migration */
#define CONFIG_CORE_ANY             ((1U << CONFIG_NUM_CORES) - 1)

/** Stack size per core (64 KB) */
#define CONFIG_CORE_STACK_SIZE      (64 * 1024)

/* -----------------------------------------------------------------------------
 * Kernel Configuration
 * ---------------------------------------------------------------------------*/

/** Maximum number of tasks (including idle tasks per core) */
#define CONFIG_MAX_TASKS            16

/** Number of priority levels (0 = lowest, CONFIG_MAX_PRIORITY-1 = highest) */
#define CONFIG_MAX_PRIORITY         4

/** System tick frequency in Hz */
#define CONFIG_TICK_RATE_HZ         1000

/** Minimal stack size in bytes */
#define CONFIG_MINIMAL_STACK_SIZE   1024

/** Idle task stack size in bytes */
#define CONFIG_IDLE_STACK_SIZE      1024

/* -----------------------------------------------------------------------------
 * Platform Configuration (QEMU virt machine)
 * ---------------------------------------------------------------------------*/

/** CPU frequency (QEMU virt default: 10 MHz) */
#define CONFIG_CPU_FREQ_HZ          10000000UL

/** UART base address (QEMU virt: ns16550a at 0x10000000) */
#define CONFIG_UART_BASE            0x10000000UL

/** CLINT base address (Core Local Interruptor) */
#define CONFIG_CLINT_BASE           0x02000000UL

/** CLINT mtime register offset */
#define CONFIG_CLINT_MTIME_OFFSET   0xBFF8

/** CLINT mtimecmp register offset (hart 0) */
#define CONFIG_CLINT_MTIMECMP_OFFSET 0x4000

/** CLINT MSIP (Software Interrupt) offset */
#define CONFIG_CLINT_MSIP_OFFSET    0x0000

/* -----------------------------------------------------------------------------
 * Debug Configuration
 * ---------------------------------------------------------------------------*/

/** Enable debug output */
#define CONFIG_DEBUG                0

/** Enable kernel assertions */
#define CONFIG_ASSERT               1

/* -----------------------------------------------------------------------------
 * Architecture Configuration
 * ---------------------------------------------------------------------------*/

/** RISC-V XLEN (32 or 64) */
#define CONFIG_RISCV_XLEN           64

/** Register size in bytes */
#define CONFIG_REG_SIZE             8

/** Stack alignment (RISC-V ABI requires 16-byte alignment) */
#define CONFIG_STACK_ALIGNMENT      16

#endif /* DANIERTOS_CONFIG_H */

