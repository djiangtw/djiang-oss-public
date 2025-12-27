/**
 * danieRTOS v3.x - Physical Memory Protection (PMP) Implementation
 *
 * Copyright (c) 2025 Danny Jiang <djiang.tw@gmail.com>
 * SPDX-License-Identifier: MIT
 *
 * v3p2: Pure NAPOT Strategy (more efficient than TOR)
 *
 * NAPOT advantages over TOR:
 *   - Uses 1 PMP entry per region (TOR uses 2)
 *   - Faster hardware decode (masking vs comparison)
 *   - Better for multi-task isolation
 *
 * PMP Entry Layout:
 *   Entry 0: User Code/Data   - U:RWX (Static, covers user section)
 *   Entry 1: User Stacks      - U:RW  (Static, covers all user stacks)
 *   Entry 2: Task Stack       - U:RW  (Dynamic, per-task isolation)
 *   Entry 3-7: Reserved
 *
 * Note: U-mode access is DENIED by default if no PMP entry matches.
 *       No need for a TOR catch-all entry.
 */

#include "pmp.h"
#include "daniertos.h"
#include "task.h"

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

/**
 * Round up to the next power of 2
 */
static reg_t next_power_of_2(reg_t n)
{
    if (n == 0) return 1;
    n--;
    n |= n >> 1;
    n |= n >> 2;
    n |= n >> 4;
    n |= n >> 8;
    n |= n >> 16;
    n |= n >> 32;
    return n + 1;
}

reg_t pmp_napot_addr(reg_t base, reg_t size)
{
    /*
     * NAPOT encoding for pmpaddr:
     *   pmpaddr = (base >> 2) | ((size >> 3) - 1)
     *
     * This encodes a region [base, base + size) where size MUST be a power of 2.
     * We round up size to the next power of 2 if necessary.
     *
     * Note: base must also be aligned to size for NAPOT to work correctly.
     */
    reg_t napot_size = next_power_of_2(size);
    return (base >> 2) | ((napot_size >> 3) - 1);
}

/* -----------------------------------------------------------------------------
 * PMP initialization
 * ---------------------------------------------------------------------------*/

void pmp_init(void)
{
#if CONFIG_USER_MODE
    /*
     * PMP Configuration for danieRTOS v3p2 (Pure NAPOT Strategy)
     *
     * Entry 0: User code/data region - U-mode RWX (Static)
     * Entry 1: Shared region (rodata) - U-mode R   (Static, for string constants)
     * Entry 2: User stacks region    - U-mode RW  (Static)
     * Entry 3-7: Reserved (OFF)
     *
     * NAPOT benefits:
     *   - 1 entry per region (TOR needs 2)
     *   - Faster hardware decode
     *   - U-mode denied by default (no catch-all needed)
     *
     * For SMP: Each hart calls pmp_init() at boot.
     */

    reg_t user_base = (reg_t)_pmp_user_start;
    reg_t user_size = (reg_t)_pmp_user_end - (reg_t)_pmp_user_start;
    reg_t shared_base = (reg_t)_pmp_shared_start;
    reg_t shared_size = (reg_t)_pmp_shared_end - (reg_t)_pmp_shared_start;
    reg_t stacks_base = (reg_t)_pmp_stacks_start;
    reg_t stacks_size = (reg_t)_pmp_stacks_end - (reg_t)_pmp_stacks_start;

    /* Entry 0: User code/data region - RWX for U-mode (NAPOT) */
    csr_write_pmpaddr(0, pmp_napot_addr(user_base, user_size));

    /* Entry 1: Shared region (rodata + data) - R for U-mode (NAPOT)
     * This allows U-mode to read string constants and global data */
    csr_write_pmpaddr(1, pmp_napot_addr(shared_base, shared_size));

    /* Entry 2: User stacks region - RW for U-mode (NAPOT)
     * This allows U-mode to access the general stack area */
    csr_write_pmpaddr(2, pmp_napot_addr(stacks_base, stacks_size));

    /* Entry 3-7: Reserved (disabled) */
    csr_write_pmpaddr(3, 0);

    /*
     * Configure pmpcfg0 (entries 0-7 on RV64):
     *   Entry 0: NAPOT + RWX (user code/data)
     *   Entry 1: NAPOT + R   (shared region - rodata)
     *   Entry 2: NAPOT + RW  (user stacks area)
     *   Entry 3-7: OFF (disabled)
     *
     * Note: U-mode access is DENIED by default for unmatched addresses.
     */
    reg_t cfg = 0;
    cfg |= ((reg_t)(PMP_A_NAPOT | PMP_RWX)) << (0 * 8);  /* Entry 0: User code */
    cfg |= ((reg_t)(PMP_A_NAPOT | PMP_R))   << (1 * 8);  /* Entry 1: Shared (R) */
    cfg |= ((reg_t)(PMP_A_NAPOT | PMP_RW))  << (2 * 8);  /* Entry 2: User stacks */
    cfg |= ((reg_t)(PMP_A_OFF))             << (3 * 8);  /* Entry 3: Disabled */

    csr_write_pmpcfg(0, cfg);

    /* Fence to ensure PMP changes take effect */
    __asm__ volatile("sfence.vma" ::: "memory");

#if CONFIG_DEBUG
    cpu_t *cpu = smp_get_cpu();
    uart_printf("[PMP][Hart %lu] Pure NAPOT initialized:\n", cpu->hartid);
    uart_printf("  Entry 0 (Code):   0x%lx size=0x%lx (RWX)\n", user_base, user_size);
    uart_printf("  Entry 1 (Stacks): 0x%lx size=0x%lx (RW)\n", stacks_base, stacks_size);
    uart_printf("  Entry 2 (Task):   Dynamic (per-task)\n");
#endif

#endif /* CONFIG_USER_MODE */
}

/* -----------------------------------------------------------------------------
 * Dynamic PMP Update (v3.x: called on context switch)
 * ---------------------------------------------------------------------------*/

void pmp_update_task(struct tcb *task)
{
#if CONFIG_USER_MODE
    if (task == NULL || task->ustack_base == NULL || task->ustack_size == 0) {
        /* No valid user stack - disable entry 2 */
        csr_write_pmpaddr(2, 0);
        return;
    }

    /*
     * Update PMP Entry 2 to protect current task's user stack.
     * This is called from switch_to() after selecting the new task.
     *
     * Note: ustack_size must be a power of 2 for NAPOT encoding.
     * The task creation code should ensure this.
     */
    reg_t stack_base = (reg_t)task->ustack_base;
    reg_t stack_size = (reg_t)task->ustack_size;

    csr_write_pmpaddr(2, pmp_napot_addr(stack_base, stack_size));

    /* Fence to ensure PMP change takes effect before returning to user */
    __asm__ volatile("sfence.vma" ::: "memory");

#if CONFIG_DEBUG > 1
    cpu_t *cpu = smp_get_cpu();
    uart_printf("[PMP][Hart %lu] Updated for task %s: stack 0x%lx-0x%lx\n",
                cpu->hartid, task->name, stack_base, stack_base + stack_size);
#endif

#else
    (void)task;  /* Unused when CONFIG_USER_MODE is disabled */
#endif
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

