/**
 * danieRTOS - Producer/Consumer Demo
 *
 * Copyright (c) 2025 Danny Jiang <djiang.tw@gmail.com>
 * SPDX-License-Identifier: MIT
 *
 * Classic producer/consumer pattern:
 * - Blinker task (heartbeat)
 * - Producer (sends messages when signaled)
 * - Consumer (signals producer, receives messages)
 */

#include "daniertos.h"

/* Task stacks */
static uint8_t stack1[CONFIG_MINIMAL_STACK_SIZE] __attribute__((aligned(16)));
static uint8_t stack2[CONFIG_MINIMAL_STACK_SIZE] __attribute__((aligned(16)));
static uint8_t stack3[CONFIG_MINIMAL_STACK_SIZE] __attribute__((aligned(16)));

/* Synchronization */
static semaphore_t sem;
static queue_t msg_queue;
static uint32_t queue_buf[8];

static void task_blinker(void *arg)
{
    (void)arg;
    int count = 0;

    while (1) {
        log_printf("Blink #%d\n", count++);
        delay_ms(2000);
    }
}

static void task_producer(void *arg)
{
    (void)arg;
    uint32_t msg = 0;

    while (1) {
        sem_wait(&sem, WAIT_FOREVER);

        if (queue_send(&msg_queue, &msg, tick_from_ms(100))) {
            log_printf("Producer: sent %lu\n", msg);
            msg++;
        }

        delay_ms(100);
    }
}

static void task_consumer(void *arg)
{
    (void)arg;
    uint32_t msg;
    int sig_count = 0;

    while (1) {
        sig_count++;

        /* Signal producer every 2 seconds */
        if ((sig_count % 4) == 0) {
            log_printf("Consumer: signal #%d\n", sig_count / 4);
            sem_signal(&sem);
        }

        /* Try to receive */
        if (queue_receive(&msg_queue, &msg, tick_from_ms(200))) {
            log_printf("Consumer: got %lu\n", msg);
        }

        delay_ms(500);
    }
}

void demo_init(void)
{
    sem_init(&sem, 0);
    queue_init(&msg_queue, queue_buf, sizeof(uint32_t), 8);

    task_create("blinker",  task_blinker,  NULL, 1, stack1, sizeof(stack1));
    task_create("producer", task_producer, NULL, 2, stack2, sizeof(stack2));
    task_create("consumer", task_consumer, NULL, 3, stack3, sizeof(stack3));

    uart_printf("Producer/Consumer Demo - 3 tasks\n\n");
}

