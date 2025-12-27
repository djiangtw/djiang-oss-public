/**
 * danieRTOS v3.x - Trap Handler (C part, SMP + User Mode Edition)
 *
 * Copyright (c) 2025 Danny Jiang <djiang.tw@gmail.com>
 * SPDX-License-Identifier: MIT
 */

#include "daniertos.h"
#include "smp.h"
#include "syscall.h"

/* Exception cause codes */
#define EXCEPTION_ECALL_U   8   /* ecall from U-mode */
#define EXCEPTION_ECALL_S   9   /* ecall from S-mode */
#define EXCEPTION_ECALL_M   11  /* ecall from M-mode */

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
            /* ecall from M/S mode - used for yield (v2 compatibility) */
            /* Advance mepc past the ecall instruction (4 bytes) */
            sp[32] += 4;  /* mepc is at index 32 in context frame */
            break;

        default:
            /* Other exception - fault handling */
            {
                cpu_t *cpu = smp_get_cpu();
                reg_t mstatus = sp[33];  /* mstatus from context */
#if CONFIG_USER_MODE
                reg_t mpp = (mstatus >> 11) & 3;
                uart_printf("[Fault][Core %d] mcause=%ld, mtval=0x%lx, mepc=0x%lx (mode=%s)\n",
                            cpu->hartid, cause, csr_read(mtval), csr_read(mepc),
                            mpp == 3 ? "M" : "U");

                if (mpp == 0 && cpu->current_task != NULL) {
                    /* ==========================================================
                     * v3p2 Fault Isolation: User task fault - terminate task
                     * ==========================================================*/
                    uart_printf("[Fault] Terminating user task '%s'\n",
                                cpu->current_task->name);

                    /* Clean up and terminate the faulting task */
                    sched_exit_isr();
                    task_exit();  /* Mark task as deleted */
                    return sched_schedule(sp);  /* Switch to another task */
                }

                /* M-mode exception - this is a kernel bug, halt */
                uart_printf("[PANIC] Kernel exception! Task: %s\n",
                            cpu->current_task ? cpu->current_task->name : "(null)");
#else
                (void)mstatus;
                uart_printf("[Core %d] Exception: mcause=%lx, mtval=%lx, mepc=%lx\n",
                            cpu->hartid, mcause, csr_read(mtval), csr_read(mepc));
                uart_printf("[Core %d] current_task=%s, sp=%p\n",
                            cpu->hartid,
                            cpu->current_task ? cpu->current_task->name : "(null)",
                            sp);
#endif
                while (1) {
                    asm volatile ("wfi");
                }
            }
        }
    }

    /* Check if context switch is needed */
    if (sched_switch_pending()) {
        sp = sched_schedule(sp);
    }

    sched_exit_isr();



#if !CONFIG_USER_MODE
    /* Sanity check: mstatus should have valid MPP field (M-mode only build) */
    reg_t final_mstatus = sp[33];
    if ((final_mstatus & 0x1800) == 0) {
        /* MPP=00 means User mode - but we always run in M-mode in v2 builds! */
        uart_printf("[Core %d] FATAL: invalid mstatus=%lx in context, mepc=%lx\n",
                    smp_get_hartid(), final_mstatus, sp[32]);
        while(1) { asm volatile("wfi"); }
    }
#endif

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

