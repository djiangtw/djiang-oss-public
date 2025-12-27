/**
 * danieRTOS - UART Driver (NS16550A for QEMU virt)
 *
 * Copyright (c) 2025 Danny Jiang <djiang.tw@gmail.com>
 * SPDX-License-Identifier: MIT
 */

#include "hal.h"
#include "config.h"
#include <stdarg.h>

/* Simple spinlock for UART output to prevent interleaved prints */
static volatile int uart_lock = 0;

static inline uint64_t uart_acquire(void)
{
    /* Disable interrupts while holding lock to prevent deadlock */
    uint64_t mstatus;
    asm volatile ("csrrci %0, mstatus, 0x8" : "=r"(mstatus));  /* Clear MIE */

    while (__sync_lock_test_and_set(&uart_lock, 1)) {
        /* spin */
    }
    return mstatus;
}

static inline void uart_release(uint64_t mstatus)
{
    __sync_lock_release(&uart_lock);

    /* Restore previous interrupt state */
    if (mstatus & 0x8) {
        asm volatile ("csrsi mstatus, 0x8");  /* Set MIE if it was set */
    }
}

/* -----------------------------------------------------------------------------
 * NS16550A Register Offsets
 * ---------------------------------------------------------------------------*/
#define UART_RBR    0   /* Receive Buffer Register (read) */
#define UART_THR    0   /* Transmit Holding Register (write) */
#define UART_IER    1   /* Interrupt Enable Register */
#define UART_FCR    2   /* FIFO Control Register (write) */
#define UART_ISR    2   /* Interrupt Status Register (read) */
#define UART_LCR    3   /* Line Control Register */
#define UART_MCR    4   /* Modem Control Register */
#define UART_LSR    5   /* Line Status Register */
#define UART_MSR    6   /* Modem Status Register */
#define UART_SCR    7   /* Scratch Register */

/* Line Status Register bits */
#define UART_LSR_RX_READY   0x01    /* Data ready */
#define UART_LSR_TX_EMPTY   0x20    /* THR empty */

/* -----------------------------------------------------------------------------
 * UART Access
 * ---------------------------------------------------------------------------*/
static volatile uint8_t *const uart = (volatile uint8_t *)CONFIG_UART_BASE;

#define uart_read(reg)      (uart[reg])
#define uart_write(reg, v)  (uart[reg] = (v))

/* -----------------------------------------------------------------------------
 * UART Functions
 * ---------------------------------------------------------------------------*/

void uart_init(void)
{
    /* Disable interrupts */
    uart_write(UART_IER, 0x00);

    /* Enable FIFO, clear buffers */
    uart_write(UART_FCR, 0x07);

    /* 8 bits, no parity, 1 stop bit */
    uart_write(UART_LCR, 0x03);

    /* Enable DTR, RTS */
    uart_write(UART_MCR, 0x03);
}

void uart_putc(char c)
{
    /* Wait until TX buffer is empty */
    while ((uart_read(UART_LSR) & UART_LSR_TX_EMPTY) == 0)
        ;
    uart_write(UART_THR, c);
}

/* Internal: puts without locking (for use inside printf) */
static void uart_puts_unlocked(const char *s)
{
    while (*s) {
        if (*s == '\n')
            uart_putc('\r');
        uart_putc(*s++);
    }
}

void uart_puts(const char *s)
{
    uint64_t mstatus = uart_acquire();
    uart_puts_unlocked(s);
    uart_release(mstatus);
}

char uart_getc(void)
{
    /* Wait until data is ready */
    while ((uart_read(UART_LSR) & UART_LSR_RX_READY) == 0)
        ;
    return uart_read(UART_RBR);
}

int uart_getc_nonblock(void)
{
    if (uart_read(UART_LSR) & UART_LSR_RX_READY)
        return uart_read(UART_RBR);
    return -1;
}

/* Internal: puthex without locking (for use inside printf) */
static void uart_puthex_unlocked(uint64_t val)
{
    static const char hex[] = "0123456789abcdef";
    uart_putc('0');
    uart_putc('x');
    for (int i = 60; i >= 0; i -= 4) {
        uart_putc(hex[(val >> i) & 0xF]);
    }
}

void uart_puthex(uint64_t val)
{
    uint64_t mstatus = uart_acquire();
    uart_puthex_unlocked(val);
    uart_release(mstatus);
}

void uart_putdec(int64_t val)
{
    char buf[21];
    int i = 0;
    bool neg = false;

    if (val < 0) {
        neg = true;
        val = -val;
    }

    if (val == 0) {
        uart_putc('0');
        return;
    }

    while (val > 0) {
        buf[i++] = '0' + (val % 10);
        val /= 10;
    }

    if (neg)
        uart_putc('-');

    while (i > 0)
        uart_putc(buf[--i]);
}

void uart_printf(const char *fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);

    uint64_t mstatus = uart_acquire();  /* Lock to prevent interleaved output */

    while (*fmt) {
        if (*fmt == '%') {
            fmt++;
            /* Skip width specifier (e.g., %5d, %10s) */
            while (*fmt >= '0' && *fmt <= '9') {
                fmt++;
            }
            switch (*fmt) {
            case 'd':
            case 'i':
                uart_putdec(va_arg(ap, int));
                break;
            case 'l':
                fmt++;
                if (*fmt == 'd') {
                    uart_putdec(va_arg(ap, int64_t));
                } else if (*fmt == 'u') {
                    uart_putdec((int64_t)va_arg(ap, uint64_t));
                } else if (*fmt == 'x') {
                    uart_puthex_unlocked(va_arg(ap, uint64_t));
                }
                break;
            case 'u':
                uart_putdec((int64_t)(uint32_t)va_arg(ap, unsigned int));
                break;
            case 'x':
            case 'p':
                uart_puthex_unlocked(va_arg(ap, uint64_t));
                break;
            case 's':
                uart_puts_unlocked(va_arg(ap, const char *));
                break;
            case 'c':
                uart_putc((char)va_arg(ap, int));
                break;
            case '%':
                uart_putc('%');
                break;
            default:
                uart_putc('%');
                uart_putc(*fmt);
                break;
            }
        } else {
            if (*fmt == '\n')
                uart_putc('\r');
            uart_putc(*fmt);
        }
        fmt++;
    }

    uart_release(mstatus);  /* Unlock */

    va_end(ap);
}

/* External tick function - defined in tick.c */
extern uint64_t tick_get(void);

void log_printf(const char *fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);

    uint64_t mstatus = uart_acquire();

    /* Print timestamp */
    uart_putc('[');
    uart_putdec((int64_t)tick_get());
    uart_putc(']');
    uart_putc(' ');

    /* Print message */
    while (*fmt) {
        if (*fmt == '%') {
            fmt++;
            /* Skip width specifier (e.g., %5d, %10s) */
            while (*fmt >= '0' && *fmt <= '9') {
                fmt++;
            }
            switch (*fmt) {
            case 'd':
            case 'i':
                uart_putdec(va_arg(ap, int));
                break;
            case 'l':
                fmt++;
                if (*fmt == 'd') {
                    uart_putdec(va_arg(ap, int64_t));
                } else if (*fmt == 'u') {
                    uart_putdec((int64_t)va_arg(ap, uint64_t));
                } else if (*fmt == 'x') {
                    uart_puthex_unlocked(va_arg(ap, uint64_t));
                }
                break;
            case 'u':
                uart_putdec((int64_t)(uint32_t)va_arg(ap, unsigned int));
                break;
            case 'x':
            case 'p':
                uart_puthex_unlocked(va_arg(ap, uint64_t));
                break;
            case 's':
                uart_puts_unlocked(va_arg(ap, const char *));
                break;
            case 'c':
                uart_putc((char)va_arg(ap, int));
                break;
            case '%':
                uart_putc('%');
                break;
            default:
                uart_putc('%');
                uart_putc(*fmt);
                break;
            }
        } else {
            if (*fmt == '\n')
                uart_putc('\r');
            uart_putc(*fmt);
        }
        fmt++;
    }

    uart_release(mstatus);

    va_end(ap);
}
