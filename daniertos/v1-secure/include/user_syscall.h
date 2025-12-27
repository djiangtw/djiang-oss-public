/**
 * danieRTOS v1.x - User-mode System Call API
 *
 * Copyright (c) 2025 Danny Jiang <djiang.tw@gmail.com>
 * SPDX-License-Identifier: MIT
 *
 * This header is for user tasks running in U-mode.
 * These functions execute ecall to trap into M-mode kernel.
 *
 * IMPORTANT: This file and its implementation (user_syscall.c) must be
 * linked into the USER region (0x80020000+) so U-mode can execute them.
 */

#ifndef DANIERTOS_USER_SYSCALL_H
#define DANIERTOS_USER_SYSCALL_H

#include "types.h"
#include "syscall.h"  /* For syscall numbers */

/* -----------------------------------------------------------------------------
 * User-mode API
 *
 * These are thin wrappers that execute ecall instruction.
 * They run in U-mode and can be called by user tasks.
 * ---------------------------------------------------------------------------*/

/**
 * Yield CPU to other ready tasks
 */
void sys_yield(void);

/**
 * Delay for specified number of ticks
 * @param ticks Number of ticks to delay (1 tick = 1ms with default config)
 */
void sys_delay(tick_t ticks);

/**
 * Delay for specified number of milliseconds
 * @param ms Number of milliseconds to delay
 */
static inline void sys_delay_ms(uint32_t ms)
{
    sys_delay((tick_t)ms);  /* At 1000 Hz, 1 tick = 1ms */
}

/**
 * Get current system tick count
 * @return Current tick count
 */
tick_t sys_get_tick(void);

/**
 * Exit current task
 */
void sys_exit(void) __attribute__((noreturn));

/**
 * Output a character (via kernel UART)
 * @param c Character to output
 */
void sys_putchar(char c);

/**
 * Output a string (via kernel UART)
 * @param s Null-terminated string to output
 */
void sys_puts(const char *s);

/**
 * Get current task ID
 * @return Task ID
 */
uint8_t sys_get_task_id(void);

/* -----------------------------------------------------------------------------
 * Formatted output (simplified printf for user mode)
 *
 * Supports: %d, %u, %x, %s, %c, %%
 * ---------------------------------------------------------------------------*/

/**
 * Simple printf for user tasks
 * @param fmt Format string
 * @param ... Arguments
 */
void sys_printf(const char *fmt, ...) __attribute__((format(printf, 1, 2)));

/**
 * Printf with timestamp prefix for user tasks (dmesg-like format)
 * Output format: [SSSS.MMM] message
 * Where SSSS = seconds, MMM = milliseconds
 *
 * Example: [0001.500] Task A started
 *
 * @param fmt Format string
 * @param ... Arguments
 */
void sys_log(const char *fmt, ...) __attribute__((format(printf, 1, 2)));

#endif /* DANIERTOS_USER_SYSCALL_H */

