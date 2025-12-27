/**
 * danieRTOS v1.x - BLE Temperature Sensor Demo
 *
 * Copyright (c) 2025 Danny Jiang <djiang.tw@gmail.com>
 * SPDX-License-Identifier: MIT
 *
 * Simulates a BLE IoT device with:
 * - Heartbeat LED (2 second toggle)
 * - Temperature sensor (2 second readings)
 * - BLE transmitter (periodic advertising)
 *
 * v1.x: User tasks run in U-mode using syscalls.
 */

#include "daniertos.h"
#include "user_syscall.h"

/* Task stacks */
static uint8_t stack1[CONFIG_MINIMAL_STACK_SIZE] __attribute__((aligned(16)));
static uint8_t stack2[CONFIG_MINIMAL_STACK_SIZE] __attribute__((aligned(16)));
static uint8_t stack3[CONFIG_MINIMAL_STACK_SIZE] __attribute__((aligned(16)));

static uint32_t rand_state = 12345;

static uint32_t rand_next(void)
{
    rand_state = rand_state * 1103515245 + 12345;
    return (rand_state >> 16) % 100;
}

/* Shared temperature for BLE to read */
static volatile int current_temp = 22;
static volatile int temp_changed = 0;

/* -----------------------------------------------------------------------------
 * User Task Functions (run in U-mode)
 * ---------------------------------------------------------------------------*/

static void task_heartbeat(void *arg)
{
    (void)arg;
    int led_on = 0;

    while (1) {
        led_on = !led_on;
        sys_log("LED %s\n", led_on ? "ON " : "OFF");
        sys_delay_ms(2000);
    }
}

static void task_sensor(void *arg)
{
    (void)arg;
    int temp = 22;
    int last_temp = 22;
    int cycle = 0;

    while (1) {
        cycle++;

        /* 20% up, 20% down, 60% same */
        uint32_t r = rand_next();
        if (r < 20 && temp < 30) {
            temp++;
        } else if (r < 40 && temp > 15) {
            temp--;
        }

        /* Debug mode: cycles 8-10 of every 10 (=16-20s of every 20s) */
        bool debug_mode = ((cycle % 10) >= 8);

        if (debug_mode) {
            sys_log("Sensor[DBG]: %d C\n", temp);
            current_temp = temp;
            temp_changed = 1;
        } else if (temp != last_temp) {
            sys_log("Sensor: %d C\n", temp);
            current_temp = temp;
            temp_changed = 1;
            last_temp = temp;
        }

        sys_delay_ms(2000);
    }
}

static void task_ble_tx(void *arg)
{
    (void)arg;
    int tx_count = 0;

    while (1) {
        if (temp_changed) {
            temp_changed = 0;
            tx_count++;
            sys_log("BLE TX #%d: %d C\n", tx_count, current_temp);
        } else {
            sys_log("BLE: advertising...\n");
        }
        sys_delay_ms(5000);  /* Check every 5 seconds */
    }
}

/* -----------------------------------------------------------------------------
 * Demo Initialization (runs in M-mode)
 * ---------------------------------------------------------------------------*/

void demo_init(void)
{
    task_create("heartbeat", task_heartbeat, NULL, 1, stack1, sizeof(stack1));
    task_create("sensor",    task_sensor,    NULL, 2, stack2, sizeof(stack2));
    task_create("ble_tx",    task_ble_tx,    NULL, 3, stack3, sizeof(stack3));

    uart_printf("BLE Temp Sensor Demo - 3 tasks (U-mode)\n\n");
}

