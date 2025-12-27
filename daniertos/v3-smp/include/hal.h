/**
 * danieRTOS - Hardware Abstraction Layer
 *
 * Copyright (c) 2025 Danny Jiang <djiang.tw@gmail.com>
 * SPDX-License-Identifier: MIT
 */

#ifndef DANIERTOS_HAL_H
#define DANIERTOS_HAL_H

#include "types.h"

/* -----------------------------------------------------------------------------
 * UART
 * ---------------------------------------------------------------------------*/

void uart_init(void);
void uart_putc(char c);
void uart_puts(const char *s);
void uart_puthex(uint64_t val);
void uart_putdec(int64_t val);
char uart_getc(void);
int  uart_getc_nonblock(void);

/* Simple printf-like function */
void uart_printf(const char *fmt, ...);

/* Printf with timestamp prefix: "[tick] msg" */
void log_printf(const char *fmt, ...);

/* -----------------------------------------------------------------------------
 * Timer (CLINT)
 * ---------------------------------------------------------------------------*/

void     timer_init(void);
void     timer_init_ap(uint64_t hartid);  /* Initialize timer for AP (core 1+) */
uint64_t timer_get_mtime(void);
void     timer_set_mtimecmp(uint64_t value);
void     timer_set_next_tick(void);

/* -----------------------------------------------------------------------------
 * Interrupts
 * ---------------------------------------------------------------------------*/

void interrupts_enable(void);
void interrupts_disable(void);
bool interrupts_enabled(void);

/* -----------------------------------------------------------------------------
 * CSR Access Macros
 * ---------------------------------------------------------------------------*/

#define csr_read(csr) ({ \
    reg_t __val; \
    asm volatile ("csrr %0, " #csr : "=r"(__val)); \
    __val; \
})

#define csr_write(csr, val) ({ \
    reg_t __val = (val); \
    asm volatile ("csrw " #csr ", %0" : : "r"(__val)); \
})

#define csr_set(csr, val) ({ \
    reg_t __val = (val); \
    asm volatile ("csrs " #csr ", %0" : : "r"(__val)); \
})

#define csr_clear(csr, val) ({ \
    reg_t __val = (val); \
    asm volatile ("csrc " #csr ", %0" : : "r"(__val)); \
})

/* -----------------------------------------------------------------------------
 * CSR Bit Definitions
 * ---------------------------------------------------------------------------*/

/* mstatus bits */
#define MSTATUS_MIE     (1UL << 3)   /* Machine Interrupt Enable */
#define MSTATUS_MPIE    (1UL << 7)   /* Previous MIE */
#define MSTATUS_MPP     (3UL << 11)  /* Previous Privilege Mode */
#define MSTATUS_MPP_BIT 11           /* MPP bit position */
#define MSTATUS_MPP_M   (3UL << 11)  /* MPP = M-mode (3) */
#define MSTATUS_MPP_U   (0UL << 11)  /* MPP = U-mode (0) */
#define MSTATUS_UXL     (3UL << 32)  /* User XLEN (RV64 only) */
#define MSTATUS_UXL_64  (2UL << 32)  /* UXL = 64-bit */
#define MSTATUS_SXL     (3UL << 34)  /* Supervisor XLEN (RV64 only) */
#define MSTATUS_SXL_64  (2UL << 34)  /* SXL = 64-bit */

/* mie bits */
#define MIE_MTIE        (1UL << 7)   /* Machine Timer Interrupt Enable */
#define MIE_MEIE        (1UL << 11)  /* Machine External Interrupt Enable */
#define MIE_MSIE        (1UL << 3)   /* Machine Software Interrupt Enable */

/* mcause values */
#define MCAUSE_INTERRUPT    (1UL << 63)
#define MCAUSE_MTI          (MCAUSE_INTERRUPT | 7)  /* Machine Timer Interrupt */
#define MCAUSE_MEI          (MCAUSE_INTERRUPT | 11) /* Machine External Interrupt */
#define MCAUSE_MSI          (MCAUSE_INTERRUPT | 3)  /* Machine Software Interrupt */

#endif /* DANIERTOS_HAL_H */

