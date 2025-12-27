/**
 * danieRTOS v3p2 - User-mode System Call API
 *
 * Copyright (c) 2025 Danny Jiang <djiang.tw@gmail.com>
 * SPDX-License-Identifier: MIT
 *
 * This header is for user tasks running in U-mode.
 * These functions execute ecall to trap into M-mode kernel.
 *
 * IMPORTANT: All user-mode functions must be placed in .user_text section
 * using the USER_FUNC macro so they are accessible via PMP.
 */

#ifndef DANIERTOS_USER_SYSCALL_H
#define DANIERTOS_USER_SYSCALL_H

#include "types.h"
#include "syscall.h"  /* For syscall numbers */

/* -----------------------------------------------------------------------------
 * User Section Macros
 *
 * Use these macros to place functions and data in user-accessible sections.
 * PMP is configured to allow U-mode access only to these sections.
 * ---------------------------------------------------------------------------*/

/**
 * USER_FUNC - Place function in .user_text section (U-mode can execute)
 * Usage: USER_FUNC static void my_user_task(void *arg) { ... }
 */
#define USER_FUNC __attribute__((section(".user_text")))

/**
 * USER_DATA - Place data in .user_data section (U-mode can read/write)
 * Usage: USER_DATA static int shared_counter = 0;
 */
#define USER_DATA __attribute__((section(".user_data")))

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
 * Semaphore Operations (v3p2)
 * ---------------------------------------------------------------------------*/

/**
 * Wait on semaphore (P operation / down / acquire)
 * @param sem           Pointer to semaphore
 * @param timeout_ticks Maximum ticks to wait (WAIT_FOREVER for infinite)
 * @return              1 if acquired, 0 if timeout
 */
int sys_sem_wait(semaphore_t *sem, uint32_t timeout_ticks);

/**
 * Signal semaphore (V operation / up / release)
 * @param sem   Pointer to semaphore
 */
void sys_sem_signal(semaphore_t *sem);

/**
 * Try to acquire semaphore without blocking
 * @param sem   Pointer to semaphore
 * @return      1 if acquired, 0 if would block
 */
int sys_sem_trywait(semaphore_t *sem);

/* -----------------------------------------------------------------------------
 * Mutex Operations (v3p2)
 * ---------------------------------------------------------------------------*/

/**
 * Lock a mutex
 * @param mtx           Pointer to mutex
 * @param timeout_ticks Maximum ticks to wait (WAIT_FOREVER for infinite)
 * @return              1 if acquired, 0 if timeout
 */
int sys_mutex_lock(mutex_t *mtx, uint32_t timeout_ticks);

/**
 * Unlock a mutex
 * @param mtx   Pointer to mutex
 */
void sys_mutex_unlock(mutex_t *mtx);

/**
 * Try to lock a mutex without blocking
 * @param mtx   Pointer to mutex
 * @return      1 if acquired, 0 if would block
 */
int sys_mutex_trylock(mutex_t *mtx);

/* -----------------------------------------------------------------------------
 * Queue Operations (v3p2)
 * ---------------------------------------------------------------------------*/

/**
 * Send data to a queue
 * @param q             Pointer to queue
 * @param data          Pointer to data to send
 * @param timeout_ticks Maximum ticks to wait if queue is full
 * @return              1 if sent, 0 if timeout
 */
int sys_queue_send(queue_t *q, const void *data, uint32_t timeout_ticks);

/**
 * Receive data from a queue
 * @param q             Pointer to queue
 * @param data          Pointer to buffer for received data
 * @param timeout_ticks Maximum ticks to wait if queue is empty
 * @return              1 if received, 0 if timeout
 */
int sys_queue_recv(queue_t *q, void *data, uint32_t timeout_ticks);

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

