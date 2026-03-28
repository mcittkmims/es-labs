/**
 * @file lab4_1_main.cpp
 * @brief Lab 4.1 — Binary Actuator Control with Signal Conditioning
 *
 * Entry point for Laboratory Work 4.1. Initializes all hardware peripherals,
 * creates the FreeRTOS shared state mutex, and spawns three preemptive tasks
 * that together implement a binary actuator control system with a three-stage
 * signal conditioning pipeline.
 *
 * ──────────────────────────────────────────────────────────────────────────
 * System overview
 * ──────────────────────────────────────────────────────────────────────────
 *
 * The user types ON or OFF in the serial terminal. The input task parses the
 * command and stores it as the "pending command". The actuator task picks up
 * that command on each 75 ms tick and runs it through BinaryConditioner:
 *   1. Saturation       — ensures the value is strictly {false, true}.
 *   2. Debouncing       — requires the value to be stable for 100 ms.
 *   3. Persistent valid — requires 3 consecutive identical samples.
 * Only after all three stages pass does the LED output change state.
 *
 * ┌──────────────────┬─────────────────┬──────┬────────────────────────────┐
 * │ Task             │ Trigger         │ Prio │ Responsibility             │
 * ├──────────────────┼─────────────────┼──────┼────────────────────────────┤
 * │ Input Reader     │ 10 ms poll      │  3   │ Parse ON/OFF from serial   │
 * ├──────────────────┼─────────────────┼──────┼────────────────────────────┤
 * │ Actuator Control │ 75 ms period    │  2   │ Conditioning + GPIO drive  │
 * ├──────────────────┼─────────────────┼──────┼────────────────────────────┤
 * │ Display/Report   │ 500 ms period   │  1   │ LCD update + STDIO report  │
 * └──────────────────┴─────────────────┴──────┴────────────────────────────┘
 */

#include "lab4_1_main.h"
#include "actuator_data.h"
#include "task_input.h"
#include "task_actuator.h"
#include "task_display.h"

#include <Arduino.h>
#include <Arduino_FreeRTOS.h>
#include <stdio.h>

#include "StdioSerial.h"

// ── Lab 4.1 public entry points ───────────────────────────────────────────────

void lab4_1Setup() {
    // ── Initialize STDIO serial at 9600 baud ─────────────────────────────
    stdioSerialInit(9600);

    // ── Print startup banner ──────────────────────────────────────────────
    printf("\r\n");
    printf("================================================\r\n");
    printf("  Lab 4.1 — Binary Actuator Control\r\n");
    printf("  Signal Conditioning Pipeline\r\n");
    printf("  FreeRTOS Preemptive Scheduler\r\n");
    printf("  Tasks: 3 | Tick: %u Hz\r\n", (unsigned int)configTICK_RATE_HZ);
    printf("================================================\r\n");
    printf("ACTUATOR:\r\n");
    printf("  Output: LED on Pin D%u (simulates relay)\r\n",
           (unsigned int)PIN_ACTUATOR_LED);
    printf("CONDITIONING PIPELINE:\r\n");
    printf("  1. Saturation:   bool -> {false, true}\r\n");
    printf("  2. Debounce:     %u ms stable window\r\n",
           (unsigned int)DEBOUNCE_PERIOD_MS);
    printf("  3. Persistence:  %u consecutive confirmations\r\n",
           (unsigned int)PERSIST_COUNT);
    printf("TIMING:\r\n");
    printf("  Input polling:  %u ms\r\n",  (unsigned int)TASK_INPUT_PERIOD_MS);
    printf("  Actuator ctrl:  %u ms\r\n",  (unsigned int)TASK_ACTUATOR_PERIOD_MS);
    printf("  Display/LCD:    %u ms\r\n",  (unsigned int)TASK_DISPLAY_PERIOD_MS);
    printf("  STDIO report:   every %u s\r\n",
           (unsigned int)(TASK_DISPLAY_PERIOD_MS * STDIO_REPORT_EVERY_N / 1000));
    printf("================================================\r\n\r\n");

    // ── Create shared state and synchronization primitives ────────────────
    actuatorDataInit();

    // ── Create FreeRTOS tasks ─────────────────────────────────────────────
    xTaskCreate(
        vTaskInputReader,
        "Input",
        TASK_INPUT_STACK,
        NULL,
        TASK_INPUT_PRIORITY,
        NULL
    );

    xTaskCreate(
        vTaskActuator,
        "Actuator",
        TASK_ACTUATOR_STACK,
        NULL,
        TASK_ACTUATOR_PRIORITY,
        NULL
    );

    xTaskCreate(
        vTaskDisplay,
        "Display",
        TASK_DISPLAY_STACK,
        NULL,
        TASK_DISPLAY_PRIORITY,
        NULL
    );

    // The FreeRTOS scheduler starts automatically after setup() returns
    // (handled by the Arduino_FreeRTOS library integration).
}

void lab4_1Loop() {
    // All application logic runs inside the three FreeRTOS tasks.
    // This function intentionally does nothing.
}
