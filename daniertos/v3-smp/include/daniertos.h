/**
 * danieRTOS v3.x - Main Header (SMP + User Mode Edition)
 *
 * Copyright (c) 2025 Danny Jiang <djiang.tw@gmail.com>
 * SPDX-License-Identifier: MIT
 *
 * v3.x = v2.x (SMP) + v1.x (User Mode)
 *
 * Include this header to get access to all danieRTOS APIs.
 */

#ifndef DANIERTOS_H
#define DANIERTOS_H

/* Configuration */
#include "config.h"

/* Type definitions */
#include "types.h"

/* Hardware abstraction */
#include "hal.h"

/* SMP support */
#include "spinlock.h"
#include "smp.h"

/* Kernel APIs */
#include "task.h"
#include "scheduler.h"
#include "tick.h"
#include "delay.h"
#include "critical.h"

/* Synchronization primitives */
#include "semaphore.h"
#include "mutex.h"
#include "queue.h"

/* User Mode support (v3.x) */
#if CONFIG_USER_MODE
#include "pmp.h"
#include "syscall.h"
#endif

/* -----------------------------------------------------------------------------
 * Kernel Control
 * ---------------------------------------------------------------------------*/

/**
 * Initialize the kernel
 *
 * Must be called before creating tasks or using any kernel services.
 */
void kernel_init(void);

/**
 * Start the kernel
 *
 * Starts the scheduler. This function does not return.
 * At least one task must be created before calling this.
 */
void kernel_start(void);

/* -----------------------------------------------------------------------------
 * Debug/Panic
 * ---------------------------------------------------------------------------*/

#if CONFIG_ASSERT
    #define KERNEL_ASSERT(cond) \
        do { if (!(cond)) kernel_panic("ASSERT: " #cond); } while(0)
#else
    #define KERNEL_ASSERT(cond) ((void)0)
#endif

#if CONFIG_DEBUG
    #define KERNEL_DEBUG(fmt, ...) uart_printf("[KERNEL] " fmt "\n", ##__VA_ARGS__)
#else
    #define KERNEL_DEBUG(fmt, ...) ((void)0)
#endif

/**
 * Kernel panic - halt the system
 */
void kernel_panic(const char *msg);

#endif /* DANIERTOS_H */

