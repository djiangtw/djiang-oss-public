/**
 * danieRTOS v3p2 - User-mode System Call Wrappers
 *
 * Copyright (c) 2025 Danny Jiang <djiang.tw@gmail.com>
 * SPDX-License-Identifier: MIT
 *
 * This file contains user-mode syscall wrappers that execute ecall.
 * All functions are placed in .user_text section so they are accessible
 * from U-mode via PMP configuration.
 *
 * IMPORTANT: This file must NOT include any kernel headers that pull in
 * kernel function calls. Only include types and syscall numbers.
 */

#include "user_syscall.h"
#include <stdarg.h>

/* Place all user syscall functions in .user_text section */
#define USER_FUNC __attribute__((section(".user_text")))

/* -----------------------------------------------------------------------------
 * Low-level ecall wrappers
 *
 * Convention: a7 = syscall number, a0-a5 = arguments, a0 = return value
 * ---------------------------------------------------------------------------*/

static inline reg_t ecall0(reg_t num)
{
    register reg_t a0 __asm__("a0");
    register reg_t a7 __asm__("a7") = num;
    __asm__ volatile("ecall" : "=r"(a0) : "r"(a7) : "memory");
    return a0;
}

static inline reg_t ecall1(reg_t num, reg_t arg0)
{
    register reg_t a0 __asm__("a0") = arg0;
    register reg_t a7 __asm__("a7") = num;
    __asm__ volatile("ecall" : "+r"(a0) : "r"(a7) : "memory");
    return a0;
}

static inline reg_t ecall2(reg_t num, reg_t arg0, reg_t arg1)
{
    register reg_t a0 __asm__("a0") = arg0;
    register reg_t a1 __asm__("a1") = arg1;
    register reg_t a7 __asm__("a7") = num;
    __asm__ volatile("ecall" : "+r"(a0) : "r"(a1), "r"(a7) : "memory");
    return a0;
}

static inline reg_t ecall3(reg_t num, reg_t arg0, reg_t arg1, reg_t arg2)
{
    register reg_t a0 __asm__("a0") = arg0;
    register reg_t a1 __asm__("a1") = arg1;
    register reg_t a2 __asm__("a2") = arg2;
    register reg_t a7 __asm__("a7") = num;
    __asm__ volatile("ecall" : "+r"(a0) : "r"(a1), "r"(a2), "r"(a7) : "memory");
    return a0;
}

/* -----------------------------------------------------------------------------
 * User-mode API Implementation
 *
 * All functions use USER_FUNC to place them in .user_text section,
 * which is protected by PMP for U-mode access.
 * ---------------------------------------------------------------------------*/

USER_FUNC void sys_yield(void)
{
    ecall0(SYS_YIELD);
}

USER_FUNC void sys_delay(tick_t ticks)
{
    ecall1(SYS_DELAY, ticks);
}

USER_FUNC tick_t sys_get_tick(void)
{
    return ecall0(SYS_GET_TICK);
}

USER_FUNC void sys_exit(void)
{
    ecall0(SYS_EXIT);
    __builtin_unreachable();
}

USER_FUNC void sys_putchar(char c)
{
    ecall1(SYS_PUTCHAR, c);
}

USER_FUNC void sys_puts(const char *s)
{
    ecall1(SYS_PUTS, (reg_t)s);
}

USER_FUNC uint8_t sys_get_task_id(void)
{
    return (uint8_t)ecall0(SYS_GET_TASK_ID);
}

/* -----------------------------------------------------------------------------
 * Semaphore Operations (v3p2)
 * ---------------------------------------------------------------------------*/

USER_FUNC int sys_sem_wait(semaphore_t *sem, uint32_t timeout_ticks)
{
    return (int)ecall2(SYS_SEM_WAIT, (reg_t)sem, (reg_t)timeout_ticks);
}

USER_FUNC void sys_sem_signal(semaphore_t *sem)
{
    ecall1(SYS_SEM_SIGNAL, (reg_t)sem);
}

USER_FUNC int sys_sem_trywait(semaphore_t *sem)
{
    return (int)ecall1(SYS_SEM_TRYWAIT, (reg_t)sem);
}

/* -----------------------------------------------------------------------------
 * Mutex Operations (v3p2)
 * ---------------------------------------------------------------------------*/

USER_FUNC int sys_mutex_lock(mutex_t *mtx, uint32_t timeout_ticks)
{
    return (int)ecall2(SYS_MUTEX_LOCK, (reg_t)mtx, (reg_t)timeout_ticks);
}

USER_FUNC void sys_mutex_unlock(mutex_t *mtx)
{
    ecall1(SYS_MUTEX_UNLOCK, (reg_t)mtx);
}

USER_FUNC int sys_mutex_trylock(mutex_t *mtx)
{
    return (int)ecall1(SYS_MUTEX_TRYLOCK, (reg_t)mtx);
}

/* -----------------------------------------------------------------------------
 * Queue Operations (v3p2)
 * ---------------------------------------------------------------------------*/

USER_FUNC int sys_queue_send(queue_t *q, const void *data, uint32_t timeout_ticks)
{
    return (int)ecall3(SYS_QUEUE_SEND, (reg_t)q, (reg_t)data, (reg_t)timeout_ticks);
}

USER_FUNC int sys_queue_recv(queue_t *q, void *data, uint32_t timeout_ticks)
{
    return (int)ecall3(SYS_QUEUE_RECV, (reg_t)q, (reg_t)data, (reg_t)timeout_ticks);
}

/* -----------------------------------------------------------------------------
 * Simple printf implementation for user mode
 *
 * This is a minimal printf that only uses sys_putchar syscall.
 * Supports: %d, %u, %x, %s, %c, %%
 * ---------------------------------------------------------------------------*/

USER_FUNC static void put_uint(uint64_t val, int base, int min_width)
{
    char buf[20];
    int i = 0;

    if (val == 0) {
        buf[i++] = '0';
    } else {
        while (val > 0) {
            int digit = val % base;
            buf[i++] = (digit < 10) ? ('0' + digit) : ('a' + digit - 10);
            val /= base;
        }
    }

    /* Pad with zeros if needed */
    while (i < min_width) {
        buf[i++] = '0';
    }

    /* Print in reverse */
    while (i > 0) {
        sys_putchar(buf[--i]);
    }
}

USER_FUNC static void put_int(int64_t val)
{
    if (val < 0) {
        sys_putchar('-');
        val = -val;
    }
    put_uint((uint64_t)val, 10, 0);
}

USER_FUNC static void vprintf_impl(const char *fmt, va_list args)
{
    while (*fmt) {
        if (*fmt != '%') {
            sys_putchar(*fmt++);
            continue;
        }

        fmt++;  /* Skip '%' */

        switch (*fmt) {
        case 'd':
            put_int(va_arg(args, int));
            break;
        case 'u':
            put_uint(va_arg(args, unsigned int), 10, 0);
            break;
        case 'x':
            put_uint(va_arg(args, unsigned int), 16, 0);
            break;
        case 's': {
            const char *s = va_arg(args, const char *);
            if (s) sys_puts(s);
            break;
        }
        case 'c':
            sys_putchar((char)va_arg(args, int));
            break;
        case '%':
            sys_putchar('%');
            break;
        default:
            sys_putchar('%');
            sys_putchar(*fmt);
            break;
        }
        fmt++;
    }
}

USER_FUNC void sys_printf(const char *fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    vprintf_impl(fmt, args);
    va_end(args);
}

USER_FUNC void sys_log(const char *fmt, ...)
{
    /*
     * Print timestamp in dmesg-like format: [SSSS.MMMMM]
     * Where SSSS = seconds (4+ digits), MMMMM = milliseconds (3 digits, zero-padded)
     *
     * At 1000 Hz tick rate: 1 tick = 1 ms
     */
    tick_t tick = sys_get_tick();
    tick_t seconds = tick / 1000;
    tick_t millis = tick % 1000;

    sys_putchar('[');
    put_uint(seconds, 10, 4);   /* Seconds: at least 4 digits */
    sys_putchar('.');
    put_uint(millis, 10, 3);    /* Milliseconds: 3 digits, zero-padded */
    sys_putchar(']');
    sys_putchar(' ');

    /* Print the actual message */
    va_list args;
    va_start(args, fmt);
    vprintf_impl(fmt, args);
    va_end(args);
}

