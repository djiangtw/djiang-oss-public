/**
 * danieRTOS v1.x - Configuration
 *
 * Copyright (c) 2025 Danny Jiang <djiang.tw@gmail.com>
 * SPDX-License-Identifier: MIT
 *
 * v1.x: M-mode + U-mode with memory protection
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

/** Minimal user stack size in bytes */
#define CONFIG_MINIMAL_STACK_SIZE   1024

/** Kernel stack size per task in bytes (for trap handling in M-mode) */
#define CONFIG_KERNEL_STACK_SIZE    512

/** Idle task stack size in bytes (runs in M-mode) */
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
 * M+U Mode Configuration (v1.x)
 * ---------------------------------------------------------------------------*/

/** Enable M+U mode (1 = user tasks run in U-mode, 0 = all in M-mode like v0) */
#define CONFIG_USER_MODE            1

/** Number of syscalls supported */
#define CONFIG_MAX_SYSCALLS         32

/* -----------------------------------------------------------------------------
 * PMP Configuration
 * ---------------------------------------------------------------------------*/

/** Number of PMP entries to use (max 16 on most implementations) */
#define CONFIG_PMP_ENTRIES          8

/**
 * Memory layout for PMP (QEMU virt):
 *   Region 0: Kernel code/data   (M-mode only)
 *   Region 1: User code/data     (U-mode RWX)
 *   Region 2: Shared read-only   (U-mode R)
 *   Region 3: Peripherals        (M-mode only)
 */

/* -----------------------------------------------------------------------------
 * Debug Configuration
 * ---------------------------------------------------------------------------*/

/** Enable debug output */
#define CONFIG_DEBUG                0

/** Enable kernel assertions */
#define CONFIG_ASSERT               1

/** Enable syscall tracing */
#define CONFIG_SYSCALL_TRACE        0

/* -----------------------------------------------------------------------------
 * Architecture Configuration
 * ---------------------------------------------------------------------------*/

/** RISC-V XLEN (32 or 64) */
#define CONFIG_RISCV_XLEN           64

/** Register size in bytes */
#define CONFIG_REG_SIZE             8

/** Stack alignment (RISC-V ABI requires 16-byte alignment) */
#define CONFIG_STACK_ALIGNMENT      16

/* -----------------------------------------------------------------------------
 * CSR Bit Definitions (for assembly and C code)
 * ---------------------------------------------------------------------------*/

/* mstatus bits */
#define MSTATUS_MIE_BIT             3
#define MSTATUS_MPIE_BIT            7
#define MSTATUS_MPP_BIT             11
#define MSTATUS_MPP_MASK            (3UL << MSTATUS_MPP_BIT)
#define MSTATUS_MPP_M               (3UL << MSTATUS_MPP_BIT)
#define MSTATUS_MPP_U               (0UL << MSTATUS_MPP_BIT)

/* mcause interrupt bit */
#define MCAUSE_INTERRUPT_BIT        63
#define MCAUSE_INTERRUPT            (1UL << MCAUSE_INTERRUPT_BIT)

/* Exception codes */
#define EXCEPTION_ECALL_U           8
#define EXCEPTION_ECALL_S           9
#define EXCEPTION_ECALL_M           11

#endif /* DANIERTOS_CONFIG_H */

