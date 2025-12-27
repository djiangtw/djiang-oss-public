/**
 * danieRTOS v1.x - Trap Handler (C part)
 *
 * Copyright (c) 2025 Danny Jiang <djiang.tw@gmail.com>
 * SPDX-License-Identifier: MIT
 *
 * v1.x: Added syscall handling for U-mode tasks
 */

#include "daniertos.h"
#include "syscall.h"
#include "config.h"

/* External scheduler functions */
extern void sched_enter_isr(void);
extern void sched_exit_isr(void);
extern bool sched_switch_pending(void);

/**
 * C trap handler - called from trap.S
 *
 * @param sp  Pointer to saved context on stack
 * @return    New stack pointer (for context switch)
 */
reg_t *trap_handler(reg_t *sp)
{
    reg_t mcause = csr_read(mcause);
    (void)csr_read(mtval);  /* Read but unused for now */

    sched_enter_isr();

    if (mcause & MCAUSE_INTERRUPT) {
        /* ==================================================================
         * Interrupt handling (same as v0)
         * ==================================================================*/
        reg_t cause = mcause & 0xFF;

        switch (cause) {
        case 7:  /* Machine Timer Interrupt */
            /* Acknowledge by setting next timer compare */
            timer_set_next_tick();

            /* Increment system tick */
            tick_increment();
            break;

        case 11: /* Machine External Interrupt */
            /* Handle external interrupt (PLIC) - not implemented yet */
            break;

        case 3:  /* Machine Software Interrupt */
            /* Could be used for IPI or yield */
            break;

        default:
            uart_printf("Unknown interrupt: %ld\n", cause);
            break;
        }
    } else {
        /* ==================================================================
         * Exception handling
         * ==================================================================*/
        reg_t cause = mcause & 0xFF;

        switch (cause) {
#if CONFIG_USER_MODE
        case EXCEPTION_ECALL_U:
            /* -- ecall from U-mode: dispatch to syscall handler -- */
            sched_exit_isr();  /* Exit ISR context before syscall */
            return syscall_handler(sp);
#endif

        case EXCEPTION_ECALL_M:
        case EXCEPTION_ECALL_S:
            /* ecall from M/S mode - used for yield (v0 compatibility) */
            /* Advance mepc past the ecall instruction (4 bytes) */
            sp[32] += 4;  /* mepc is at index 32 in context frame */
            break;

        default:
            /* Other exception - print and halt */
            uart_printf("Exception: mcause=%lx, mtval=%lx, mepc=%lx\n",
                        mcause, csr_read(mtval), csr_read(mepc));

#if CONFIG_USER_MODE
            /* Print privilege mode info */
            reg_t mstatus = sp[33];  /* mstatus from context */
            reg_t mpp = (mstatus >> MSTATUS_MPP_BIT) & 3;
            uart_printf("Previous mode: %s\n", mpp == 3 ? "M" : "U");
#endif

            while (1) {
                asm volatile ("wfi");
            }
        }
    }

    /* Check if context switch is needed */
    if (sched_switch_pending()) {
        sp = sched_schedule(sp);
    }

    sched_exit_isr();

    return sp;
}

/**
 * Kernel panic handler
 */
void kernel_panic(const char *msg)
{
    interrupts_disable();

    uart_puts("\n\n");
    uart_puts("*** KERNEL PANIC ***\n");
    uart_puts(msg);
    uart_puts("\n");

    /* Print some diagnostic info */
    tcb_t *current = task_get_current();
    if (current) {
        uart_printf("Current task: %s (id=%d)\n", current->name, current->id);
    }

    uart_printf("mcause: %lx\n", csr_read(mcause));
    uart_printf("mepc:   %lx\n", csr_read(mepc));
    uart_printf("mtval:  %lx\n", csr_read(mtval));

    /* Halt */
    while (1) {
        asm volatile ("wfi");
    }
}

