/**
 * danieRTOS - Timer Driver (CLINT for QEMU virt)
 *
 * Copyright (c) 2025 Danny Jiang <djiang.tw@gmail.com>
 * SPDX-License-Identifier: MIT
 */

#include "hal.h"
#include "config.h"

/* -----------------------------------------------------------------------------
 * CLINT Registers
 * ---------------------------------------------------------------------------*/
#define CLINT_BASE      CONFIG_CLINT_BASE

/* mtime: 64-bit counter, increments at a fixed frequency */
#define CLINT_MTIME     (*(volatile uint64_t *)(CLINT_BASE + CONFIG_CLINT_MTIME_OFFSET))

/* mtimecmp: when mtime >= mtimecmp, timer interrupt fires */
#define CLINT_MTIMECMP  (*(volatile uint64_t *)(CLINT_BASE + CONFIG_CLINT_MTIMECMP_OFFSET))

/* Ticks per system tick */
#define TICKS_PER_TICK  (CONFIG_CPU_FREQ_HZ / CONFIG_TICK_RATE_HZ)

/* -----------------------------------------------------------------------------
 * Timer Functions
 * ---------------------------------------------------------------------------*/

void timer_init(void)
{
    /* Set first tick interrupt */
    CLINT_MTIMECMP = CLINT_MTIME + TICKS_PER_TICK;

    /* Enable machine timer interrupt */
    csr_set(mie, MIE_MTIE);
}

uint64_t timer_get_mtime(void)
{
    return CLINT_MTIME;
}

void timer_set_mtimecmp(uint64_t value)
{
    CLINT_MTIMECMP = value;
}

void timer_set_next_tick(void)
{
    /* Schedule next tick relative to current mtimecmp to avoid drift */
    CLINT_MTIMECMP += TICKS_PER_TICK;
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

