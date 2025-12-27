/**
 * danieRTOS v3p2 - Complete User Mode Demo
 *
 * Copyright (c) 2025 Danny Jiang <djiang.tw@gmail.com>
 * SPDX-License-Identifier: MIT
 *
 * This demo demonstrates v3p2 features:
 *   1. User-mode tasks with full syscall support
 *   2. Semaphore syscalls (sys_sem_wait, sys_sem_signal)
 *   3. Queue syscalls (sys_queue_send, sys_queue_recv)
 *   4. Mutex syscalls (sys_mutex_lock, sys_mutex_unlock)
 *   5. PMP memory protection
 *   6. Fault isolation
 */

#include "daniertos.h"
#include "smp.h"
#include "user_syscall.h"

/* -----------------------------------------------------------------------------
 * Shared Resources (in Kernel-accessible memory)
 *
 * Note: These are in the USER region so U-mode can access them.
 * In a real system, these would be managed by the kernel.
 * ---------------------------------------------------------------------------*/

static semaphore_t sem_ready __attribute__((section(".user_data")));
static queue_t data_queue __attribute__((section(".user_data")));
static uint32_t queue_buffer[8] __attribute__((section(".user_data")));
static mutex_t shared_mutex __attribute__((section(".user_data")));
static volatile uint32_t shared_counter __attribute__((section(".user_data")));

/* -----------------------------------------------------------------------------
 * User Task: Producer
 * Produces data and sends to queue via syscall
 * ---------------------------------------------------------------------------*/

static void producer_task(void *arg)
{
    (void)arg;
    uint32_t data = 0;

    sys_puts("[Producer] Started in U-mode\n");

    for (int i = 0; i < 5; i++) {
        /* Wait for consumer to be ready */
        sys_printf("[Producer] Waiting for consumer ready...\n");
        sys_sem_wait(&sem_ready, WAIT_FOREVER);

        /* Produce data */
        data = (uint32_t)(i * 100 + sys_get_tick());

        /* Send to queue */
        if (sys_queue_send(&data_queue, &data, 1000)) {
            sys_printf("[Producer] Sent: %u\n", data);
        } else {
            sys_puts("[Producer] Queue full, timeout!\n");
        }

        sys_delay(100);
    }

    sys_puts("[Producer] Done!\n");
    sys_exit();
}

/* -----------------------------------------------------------------------------
 * User Task: Consumer
 * Receives data from queue via syscall
 * ---------------------------------------------------------------------------*/

static void consumer_task(void *arg)
{
    (void)arg;
    uint32_t data;

    sys_puts("[Consumer] Started in U-mode\n");
    sys_delay(50);  /* Let producer start first */

    for (int i = 0; i < 5; i++) {
        /* Signal producer we're ready */
        sys_sem_signal(&sem_ready);

        /* Receive from queue */
        if (sys_queue_recv(&data_queue, &data, 2000)) {
            sys_printf("[Consumer] Received: %u\n", data);
        } else {
            sys_puts("[Consumer] Queue empty, timeout!\n");
        }

        sys_delay(150);
    }

    sys_puts("[Consumer] Done!\n");
    sys_exit();
}

/* -----------------------------------------------------------------------------
 * User Task: Counter (tests mutex)
 * Increments shared counter with mutex protection
 * ---------------------------------------------------------------------------*/

static void counter_task(void *arg)
{
    int id = (int)(uintptr_t)arg;

    sys_printf("[Counter%d] Started\n", id);

    for (int i = 0; i < 5; i++) {
        sys_mutex_lock(&shared_mutex, WAIT_FOREVER);
        uint32_t old = shared_counter;
        shared_counter = old + 1;
        sys_printf("[Counter%d] %u -> %u\n", id, old, shared_counter);
        sys_mutex_unlock(&shared_mutex);
        sys_delay(50 + id * 20);
    }

    sys_printf("[Counter%d] Done!\n", id);
    sys_exit();
}

/* -----------------------------------------------------------------------------
 * Kernel Monitor Task (M-mode)
 * ---------------------------------------------------------------------------*/

static uint8_t monitor_stack[8192] __attribute__((aligned(16)));

static void monitor_task(void *arg)
{
    (void)arg;
    uart_puts("[Monitor] Kernel monitor started\n");

    for (int i = 0; i < 8; i++) {
        uart_printf("[Monitor] tick=%d, counter=%lu, queue=%d\n",
                    (int)tick_get(), shared_counter, (int)queue_count(&data_queue));
        task_delay(200);
    }
    uart_puts("[Monitor] Done!\n");
}

/* -----------------------------------------------------------------------------
 * Demo Entry Point
 * ---------------------------------------------------------------------------*/

void demo_init(void)
{
    uart_puts("\n=== danieRTOS v3p2 Complete Demo ===\n");
    uart_printf("Features: Syscalls, Semaphore, Queue, Mutex\n\n");

    /* Initialize shared resources */
    sem_init(&sem_ready, 0);
    queue_init(&data_queue, queue_buffer, sizeof(uint32_t), 8);
    mutex_init(&shared_mutex);
    shared_counter = 0;

    /* Create M-mode monitor */
    task_create("Monitor", monitor_task, NULL, 4,
                monitor_stack, sizeof(monitor_stack), (1U << 1));

    /* Create U-mode user tasks */
    task_create_user("Producer", producer_task, NULL, 2, CONFIG_CORE_ANY);
    task_create_user("Consumer", consumer_task, NULL, 2, CONFIG_CORE_ANY);
    task_create_user("Counter1", counter_task, (void*)1, 3, CONFIG_CORE_ANY);
    task_create_user("Counter2", counter_task, (void*)2, 3, CONFIG_CORE_ANY);

    uart_puts("All tasks created. Starting scheduler...\n\n");
}

