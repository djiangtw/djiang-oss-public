/**
 * danieRTOS v1.x - Physical Memory Protection (PMP)
 *
 * Copyright (c) 2025 Danny Jiang <djiang.tw@gmail.com>
 * SPDX-License-Identifier: MIT
 *
 * PMP provides memory protection for U-mode tasks.
 * M-mode always has full access; PMP only restricts U-mode.
 */

#ifndef DANIERTOS_PMP_H
#define DANIERTOS_PMP_H

#include "types.h"

/* -----------------------------------------------------------------------------
 * PMP Configuration Bits (pmpcfg)
 *
 * Each PMP entry has an 8-bit configuration:
 *   [7]   L - Lock (makes entry apply to M-mode too, and locks it)
 *   [6:5] Reserved
 *   [4:3] A - Address matching mode
 *   [2]   X - Execute permission
 *   [1]   W - Write permission
 *   [0]   R - Read permission
 * ---------------------------------------------------------------------------*/

/* Permission bits */
#define PMP_R           (1 << 0)    /* Read */
#define PMP_W           (1 << 1)    /* Write */
#define PMP_X           (1 << 2)    /* Execute */
#define PMP_RW          (PMP_R | PMP_W)
#define PMP_RX          (PMP_R | PMP_X)
#define PMP_RWX         (PMP_R | PMP_W | PMP_X)

/* Address matching modes */
#define PMP_A_OFF       (0 << 3)    /* Disabled */
#define PMP_A_TOR       (1 << 3)    /* Top of Range */
#define PMP_A_NA4       (2 << 3)    /* Naturally aligned 4-byte */
#define PMP_A_NAPOT     (3 << 3)    /* Naturally aligned power-of-two */

/* Lock bit */
#define PMP_L           (1 << 7)    /* Lock entry */

/* -----------------------------------------------------------------------------
 * PMP API
 * ---------------------------------------------------------------------------*/

/**
 * Initialize PMP for M+U mode operation
 *
 * Sets up memory regions:
 *   - Kernel region: M-mode only
 *   - User region: U-mode RWX
 *   - Shared region: U-mode R
 *   - Stack region: U-mode RW
 *   - Peripherals: M-mode only
 */
void pmp_init(void);

/**
 * Set a PMP entry
 *
 * @param index  PMP entry index (0-15)
 * @param addr   Address (interpretation depends on mode)
 * @param cfg    Configuration byte (permissions + mode)
 */
void pmp_set_entry(unsigned int index, reg_t addr, uint8_t cfg);

/**
 * Convert address and size to NAPOT format
 *
 * NAPOT encoding: addr[XLEN-1:0] = base[XLEN-1:log2(size)] | ((size/2 - 1) >> 2)
 *
 * @param base  Base address (must be aligned to size)
 * @param size  Region size (must be power of 2, >= 8)
 * @return      NAPOT-encoded address for pmpaddr register
 */
reg_t pmp_napot_addr(reg_t base, reg_t size);

/**
 * Dump PMP configuration (for debugging)
 */
void pmp_dump(void);

#endif /* DANIERTOS_PMP_H */

