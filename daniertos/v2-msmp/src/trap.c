/**
 * danieRTOS v2.x - Trap Handler (C part, SMP Edition)
 *
 * Copyright (c) 2025 Danny Jiang <djiang.tw@gmail.com>
 * SPDX-License-Identifier: MIT
 */

#include "daniertos.h"
#include "smp.h"

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
        /* Interrupt */
        reg_t cause = mcause & 0xFF;

        switch (cause) {
        case 7:  /* Machine Timer Interrupt */
            /* Acknowledge by setting next timer compare */
            timer_set_next_tick();

            /* Increment system tick (only on core 0 to avoid double counting) */
            if (smp_get_hartid() == 0) {
                tick_increment();
            }

            /* Request reschedule on timer tick */
            sched_request_switch();
            break;

        case 11: /* Machine External Interrupt */
            /* Handle external interrupt (PLIC) - not implemented yet */
            break;

        case 3:  /* Machine Software Interrupt (IPI) */
            smp_handle_ipi();
            break;

        default:
            uart_printf("[Core %d] Unknown interrupt: %ld\n",
                        smp_get_hartid(), cause);
            break;
        }
    } else {
        /* Exception */
        reg_t cause = mcause & 0xFF;

        if (cause == 11 || cause == 9 || cause == 8) {
            /* ecall from M/S/U mode - used for yield */
            /* Advance mepc past the ecall instruction (4 bytes) */
            sp[32] += 4;  /* mepc is at index 32 in context frame */
        } else {
            /* Other exception - print and halt */
            cpu_t *cpu = smp_get_cpu();
            uart_printf("[Core %d] Exception: mcause=%lx, mtval=%lx, mepc=%lx, sp=%p\n",
                        cpu->hartid, mcause, csr_read(mtval), csr_read(mepc), sp);
            uart_printf("[Core %d] current_task=%s, mstatus_saved=%lx\n",
                        cpu->hartid,
                        cpu->current_task ? cpu->current_task->name : "(null)",
                        sp[33]);

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

    /* Sanity check: mstatus should have valid MPP field */
    reg_t final_mstatus = sp[33];
    if ((final_mstatus & 0x1800) == 0) {
        /* MPP=00 means User mode - but we always run in M-mode! */
        uart_printf("[Core %d] FATAL: invalid mstatus=%lx in context, mepc=%lx\n",
                    smp_get_hartid(), final_mstatus, sp[32]);
        while(1) { asm volatile("wfi"); }
    }

    return sp;
}

/**
 * Kernel panic handler
 */
void kernel_panic(const char *msg)
{
    interrupts_disable();

    uint64_t hartid = smp_get_hartid();

    uart_puts("\n\n");
    uart_printf("*** KERNEL PANIC (Core %d) ***\n", hartid);
    uart_puts(msg);
    uart_puts("\n");

    /* Print some diagnostic info */
    cpu_t *cpu = smp_get_cpu();
    tcb_t *current = cpu ? cpu->current_task : NULL;
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

