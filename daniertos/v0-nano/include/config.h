/**
 * danieRTOS - Configuration
 *
 * Copyright (c) 2025 Danny Jiang <djiang.tw@gmail.com>
 * SPDX-License-Identifier: MIT
 */

#ifndef DANIERTOS_CONFIG_H
#define DANIERTOS_CONFIG_H

/* -----------------------------------------------------------------------------
 * Kernel Configuration
 * ---------------------------------------------------------------------------*/

/** Maximum number of tasks (including idle task) */
#define CONFIG_MAX_TASKS            8

/** Number of priority levels (0 = lowest, CONFIG_MAX_PRIORITY-1 = highest) */
#define CONFIG_MAX_PRIORITY         4

/** System tick frequency in Hz */
#define CONFIG_TICK_RATE_HZ         1000

/** Minimal stack size in bytes */
#define CONFIG_MINIMAL_STACK_SIZE   1024

/** Idle task stack size in bytes */
#define CONFIG_IDLE_STACK_SIZE      512

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

