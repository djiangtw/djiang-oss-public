/**
 * danieRTOS v2.x - Timer Driver (CLINT for QEMU virt, SMP Edition)
 *
 * Copyright (c) 2025 Danny Jiang <djiang.tw@gmail.com>
 * SPDX-License-Identifier: MIT
 *
 * Each hart has its own mtimecmp register:
 *   mtimecmp[hartid] at CLINT_BASE + 0x4000 + hartid * 8
 */

#include "hal.h"
#include "config.h"

/* -----------------------------------------------------------------------------
 * CLINT Registers
 * ---------------------------------------------------------------------------*/
#define CLINT_BASE      CONFIG_CLINT_BASE

/* mtime: 64-bit counter, increments at a fixed frequency (shared by all harts) */
#define CLINT_MTIME     (*(volatile uint64_t *)(CLINT_BASE + CONFIG_CLINT_MTIME_OFFSET))

/* mtimecmp: per-hart, when mtime >= mtimecmp[hartid], timer interrupt fires */
#define CLINT_MTIMECMP(hartid) \
    (*(volatile uint64_t *)(CLINT_BASE + CONFIG_CLINT_MTIMECMP_OFFSET + (hartid) * 8))

/* Ticks per system tick */
#define TICKS_PER_TICK  (CONFIG_CPU_FREQ_HZ / CONFIG_TICK_RATE_HZ)

/* Get current hart ID */
static inline uint64_t get_hartid(void)
{
    uint64_t hartid;
    asm volatile("csrr %0, mhartid" : "=r"(hartid));
    return hartid;
}

/* -----------------------------------------------------------------------------
 * Timer Functions
 * ---------------------------------------------------------------------------*/

/* Forward declaration for uart_printf */
extern void uart_printf(const char *fmt, ...);

void timer_init(void)
{
    uint64_t hartid = get_hartid();

    /* Set first tick interrupt for this hart */
    CLINT_MTIMECMP(hartid) = CLINT_MTIME + TICKS_PER_TICK;

    /* Enable machine timer interrupt */
    csr_set(mie, MIE_MTIE);

    /* Enable global interrupts */
    csr_set(mstatus, MSTATUS_MIE);
}

void timer_init_ap(uint64_t hartid)
{
    /* Set first tick interrupt for this AP */
    CLINT_MTIMECMP(hartid) = CLINT_MTIME + TICKS_PER_TICK;

    /* Enable machine timer interrupt */
    csr_set(mie, MIE_MTIE);

    /* Enable global interrupts */
    csr_set(mstatus, MSTATUS_MIE);
}

uint64_t timer_get_mtime(void)
{
    return CLINT_MTIME;
}

void timer_set_mtimecmp(uint64_t value)
{
    uint64_t hartid = get_hartid();
    CLINT_MTIMECMP(hartid) = value;
}

void timer_set_next_tick(void)
{
    uint64_t hartid = get_hartid();
    /* Schedule next tick relative to current mtimecmp to avoid drift */
    CLINT_MTIMECMP(hartid) += TICKS_PER_TICK;
}

/* -----------------------------------------------------------------------------
 * Interrupt Control
 * ---------------------------------------------------------------------------*/

void interrupts_enable(void)
{
    csr_set(mstatus, MSTATUS_MIE);
}

void interrupts_disable(void)
{
    csr_clear(mstatus, MSTATUS_MIE);
}

bool interrupts_enabled(void)
{
    return (csr_read(mstatus) & MSTATUS_MIE) != 0;
}

