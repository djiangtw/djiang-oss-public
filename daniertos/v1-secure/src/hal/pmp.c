/**
 * danieRTOS v1.x - Physical Memory Protection (PMP) Implementation
 *
 * Copyright (c) 2025 Danny Jiang <djiang.tw@gmail.com>
 * SPDX-License-Identifier: MIT
 */

#include "pmp.h"
#include "daniertos.h"

/* Linker-defined symbols for memory regions */
extern char _pmp_kernel_start[];
extern char _pmp_kernel_end[];
extern char _pmp_user_start[];
extern char _pmp_user_end[];
extern char _pmp_shared_start[];
extern char _pmp_shared_end[];
extern char _pmp_stacks_start[];
extern char _pmp_stacks_end[];

/* -----------------------------------------------------------------------------
 * CSR access macros for PMP registers
 * ---------------------------------------------------------------------------*/

#define csr_write_pmpaddr(n, val) \
    __asm__ volatile("csrw pmpaddr" #n ", %0" :: "r"(val))

#define csr_read_pmpaddr(n) ({ \
    reg_t __val; \
    __asm__ volatile("csrr %0, pmpaddr" #n : "=r"(__val)); \
    __val; \
})

#define csr_write_pmpcfg(n, val) \
    __asm__ volatile("csrw pmpcfg" #n ", %0" :: "r"(val))

#define csr_read_pmpcfg(n) ({ \
    reg_t __val; \
    __asm__ volatile("csrr %0, pmpcfg" #n : "=r"(__val)); \
    __val; \
})

/* -----------------------------------------------------------------------------
 * Helper functions
 * ---------------------------------------------------------------------------*/

reg_t pmp_napot_addr(reg_t base, reg_t size)
{
    /*
     * NAPOT encoding for pmpaddr:
     *   pmpaddr = (base >> 2) | ((size >> 3) - 1)
     *
     * This encodes a region [base, base + size) where size is a power of 2.
     */
    return (base >> 2) | ((size >> 3) - 1);
}

/* -----------------------------------------------------------------------------
 * PMP initialization
 * ---------------------------------------------------------------------------*/

void pmp_init(void)
{
#if CONFIG_USER_MODE
    /*
     * PMP Configuration for danieRTOS v1.x:
     *
     * Entry 0: User code/data region - U-mode RWX
     * Entry 1: Shared region - U-mode R (read-only)
     * Entry 2: Stack region - U-mode RW
     * Entry 3: Catch-all - No U-mode access (M-mode only)
     *
     * Note: PMP entries are checked in order. First match wins.
     * The catch-all entry at the end denies U-mode access to everything else.
     */

    reg_t user_base = (reg_t)_pmp_user_start;
    reg_t user_size = (reg_t)_pmp_user_end - (reg_t)_pmp_user_start;

    reg_t shared_base = (reg_t)_pmp_shared_start;
    reg_t shared_size = (reg_t)_pmp_shared_end - (reg_t)_pmp_shared_start;

    reg_t stacks_base = (reg_t)_pmp_stacks_start;
    reg_t stacks_size = (reg_t)_pmp_stacks_end - (reg_t)_pmp_stacks_start;

    /* Entry 0: User region - RWX for U-mode */
    csr_write_pmpaddr(0, pmp_napot_addr(user_base, user_size));

    /* Entry 1: Shared region - R only for U-mode */
    csr_write_pmpaddr(1, pmp_napot_addr(shared_base, shared_size));

    /* Entry 2: Stack region - RW for U-mode */
    csr_write_pmpaddr(2, pmp_napot_addr(stacks_base, stacks_size));

    /* Entry 3: Catch-all (entire address space) - M-mode only
     * Using TOR mode with max address to cover everything else */
    csr_write_pmpaddr(3, (reg_t)-1);

    /*
     * Configure pmpcfg0 (entries 0-7 on RV64):
     *   Entry 0: NAPOT + RWX
     *   Entry 1: NAPOT + R
     *   Entry 2: NAPOT + RW
     *   Entry 3: TOR + no permissions (M-mode only)
     */
    reg_t cfg = 0;
    cfg |= ((reg_t)(PMP_A_NAPOT | PMP_RWX)) << (0 * 8);  /* Entry 0 */
    cfg |= ((reg_t)(PMP_A_NAPOT | PMP_R))   << (1 * 8);  /* Entry 1 */
    cfg |= ((reg_t)(PMP_A_NAPOT | PMP_RW))  << (2 * 8);  /* Entry 2 */
    cfg |= ((reg_t)(PMP_A_TOR))             << (3 * 8);  /* Entry 3: deny all */

    csr_write_pmpcfg(0, cfg);

    /* Fence to ensure PMP changes take effect */
    __asm__ volatile("sfence.vma" ::: "memory");

#if CONFIG_DEBUG
    uart_puts("[PMP] Initialized:\n");
    uart_printf("  User:   0x%lx - 0x%lx (RWX)\n", user_base, user_base + user_size);
    uart_printf("  Shared: 0x%lx - 0x%lx (R)\n", shared_base, shared_base + shared_size);
    uart_printf("  Stacks: 0x%lx - 0x%lx (RW)\n", stacks_base, stacks_base + stacks_size);
#endif

#endif /* CONFIG_USER_MODE */
}

void pmp_dump(void)
{
    uart_puts("[PMP] Configuration:\n");
    uart_printf("  pmpcfg0:  0x%016lx\n", csr_read_pmpcfg(0));
    uart_printf("  pmpaddr0: 0x%016lx\n", csr_read_pmpaddr(0));
    uart_printf("  pmpaddr1: 0x%016lx\n", csr_read_pmpaddr(1));
    uart_printf("  pmpaddr2: 0x%016lx\n", csr_read_pmpaddr(2));
    uart_printf("  pmpaddr3: 0x%016lx\n", csr_read_pmpaddr(3));
}

