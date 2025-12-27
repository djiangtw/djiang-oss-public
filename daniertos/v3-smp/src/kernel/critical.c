/**
 * danieRTOS - Critical Section Implementation
 *
 * Copyright (c) 2025 Danny Jiang <djiang.tw@gmail.com>
 * SPDX-License-Identifier: MIT
 */

#include "daniertos.h"

/* -----------------------------------------------------------------------------
 * Critical Section Implementation
 * ---------------------------------------------------------------------------*/

reg_t critical_enter(void)
{
    reg_t mstatus;

    /* Read current mstatus and disable interrupts atomically */
    asm volatile (
        "csrrci %0, mstatus, %1"
        : "=r"(mstatus)
        : "i"(MSTATUS_MIE)
        : "memory"
    );

    /* Return previous state (specifically the MIE bit) */
    return mstatus & MSTATUS_MIE;
}

void critical_exit(reg_t state)
{
    /* Only re-enable if interrupts were previously enabled */
    if (state) {
        asm volatile (
            "csrsi mstatus, %0"
            :
            : "i"(MSTATUS_MIE)
            : "memory"
        );
    }
}

