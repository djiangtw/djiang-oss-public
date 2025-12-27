/**
 * danieRTOS v2.x - SMP Implementation
 *
 * Copyright (c) 2025 Danny Jiang <djiang.tw@gmail.com>
 * SPDX-License-Identifier: MIT
 */

#include "daniertos.h"
#include "smp.h"
#include "spinlock.h"

/* -----------------------------------------------------------------------------
 * Per-Core Data
 * Note: Array sized for CONFIG_MAX_CORES to support compile-time configuration
 * ---------------------------------------------------------------------------*/

cpu_t g_cpus[CONFIG_MAX_CORES] __attribute__((aligned(64)));

/* -----------------------------------------------------------------------------
 * CLINT Registers for IPI
 * ---------------------------------------------------------------------------*/

#define CLINT_MSIP(hartid) \
    ((volatile uint32_t*)(CONFIG_CLINT_BASE + CONFIG_CLINT_MSIP_OFFSET + (hartid) * 4))

/* -----------------------------------------------------------------------------
 * SMP Initialization
 * ---------------------------------------------------------------------------*/

void smp_init_bsp(void)
{
    /* Initialize Core 0 data */
    cpu_t *cpu = &g_cpus[0];
    cpu->hartid = 0;
    cpu->current_task = NULL;
    cpu->idle_task = NULL;
    cpu->irq_nesting = 0;
    cpu->need_reschedule = 0;
    
    /* Set tp register to point to this core's data */
    asm volatile("mv tp, %0" :: "r"(cpu) : "memory");
    
    uart_printf("[Core 0] SMP BSP initialized, cpu=%p\n", cpu);
}

void smp_init_ap(uint64_t hartid)
{
    /* Initialize this core's data */
    cpu_t *cpu = &g_cpus[hartid];
    cpu->hartid = hartid;
    cpu->current_task = NULL;
    cpu->idle_task = NULL;
    cpu->irq_nesting = 0;
    cpu->need_reschedule = 0;
    
    /* Set tp register to point to this core's data */
    asm volatile("mv tp, %0" :: "r"(cpu) : "memory");
    
    uart_printf("[Core %d] SMP AP initialized, cpu=%p\n", hartid, cpu);
}

/* -----------------------------------------------------------------------------
 * IPI (Inter-Processor Interrupt)
 * ---------------------------------------------------------------------------*/

void smp_send_ipi(uint64_t target_hartid)
{
    if (target_hartid >= CONFIG_NUM_CORES) {
        return;
    }
    
    /* Write 1 to target hart's MSIP register */
    *CLINT_MSIP(target_hartid) = 1;
}

void smp_clear_ipi(void)
{
    uint64_t hartid = smp_get_hartid();
    
    /* Write 0 to our MSIP register */
    *CLINT_MSIP(hartid) = 0;
}

void smp_request_reschedule(uint64_t target_hartid)
{
    if (target_hartid >= CONFIG_NUM_CORES) {
        return;
    }
    
    /* Set reschedule flag */
    g_cpus[target_hartid].need_reschedule = 1;
    
    /* If target is not us, send IPI */
    if (target_hartid != smp_get_hartid()) {
        smp_send_ipi(target_hartid);
    }
}

void smp_handle_ipi(void)
{
    cpu_t *cpu = smp_get_cpu();
    
    /* Clear the IPI */
    smp_clear_ipi();
    
    /* Check if reschedule was requested */
    if (cpu->need_reschedule) {
        cpu->need_reschedule = 0;
        sched_request_switch();
    }
}

