/**
 * @file lab2_2_main.cpp
 * @brief Lab 2.2 — Button Press Duration Monitoring with FreeRTOS Preemptive Tasks
 *
 * Entry point for Laboratory Work 2.2. This file initializes all hardware
 * peripherals, creates FreeRTOS synchronization primitives (binary semaphore
 * and mutex), and spawns three preemptive tasks that together implement the
 * button press monitoring, statistics, and reporting system.
 *
 * ──────────────────────────────────────────────────────────────────────────
 * System overview
 * ──────────────────────────────────────────────────────────────────────────
 *
 * This application is a FreeRTOS port of the bare-metal Lab 2.1 system.
 * The core application logic (button FSM, statistics, reporting) is
 * preserved; only the scheduling and inter-task communication mechanisms
 * change:
 *
 * ┌──────────┬───────────┬──────┬──────────────────────────────────────────┐
 * │ Task     │ Trigger   │ Prio │ Responsibility                           │
 * ├──────────┼───────────┼──────┼──────────────────────────────────────────┤
 * │ Task 1   │ 10 ms     │  3   │ Button debounce, duration measurement    │
 * │ (Measure)│ vTaskDelay│      │ LED indicator, semaphore give            │
 * ├──────────┼───────────┼──────┼──────────────────────────────────────────┤
 * │ Task 2   │ Semaphore │  2   │ Statistics update (mutex), yellow LED    │
 * │ (Stats)  │ event     │      │ blink sequence using blocking delays     │
 * ├──────────┼───────────┼──────┼──────────────────────────────────────────┤
 * │ Task 3   │ 10 s      │  1   │ STDIO report (mutex read + reset)       │
 * │ (Report) │ DelayUntil│      │ drift-free periodic output               │
 * └──────────┴───────────┴──────┴──────────────────────────────────────────┘
 *
 * ──────────────────────────────────────────────────────────────────────────
 * Synchronization mechanisms
 * ──────────────────────────────────────────────────────────────────────────
 *
 *   Binary Semaphore (xNewPressSemaphore):
 *     Given by Task 1 when a complete press cycle is detected.
 *     Taken by Task 2 to wake up and process the event.
 *     Acts as a lightweight event flag with zero polling overhead.
 *
 *   Mutex (xSharedDataMutex):
 *     Protects g_pressInfo (Task 1 writes, Task 2 reads) and g_stats
 *     (Task 2 updates, Task 3 reads and resets). Provides priority
 *     inheritance to prevent priority inversion between tasks.
 */

#include "lab2_2_main.h"
#include "shared_state.h"
#include "task_measure.h"
#include "task_stats.h"
#include "task_report.h"

#include <Arduino.h>
#include <Arduino_FreeRTOS.h>
#include <stdio.h>

#include "StdioSerial.h"

// ──────────────────────────────────────────────────────────────────────────
// Lab 2.2 public entry points
// ──────────────────────────────────────────────────────────────────────────

void lab2_2Setup() {
    // ── Configure hardware pins ────────────────────────────────────────
    pinMode(PIN_BUTTON,     INPUT_PULLUP);
    pinMode(PIN_LED_GREEN,  OUTPUT);
    pinMode(PIN_LED_RED,    OUTPUT);
    pinMode(PIN_LED_YELLOW, OUTPUT);

    // Ensure all LEDs start in the OFF state.
    digitalWrite(PIN_LED_GREEN,  LOW);
    digitalWrite(PIN_LED_RED,    LOW);
    digitalWrite(PIN_LED_YELLOW, LOW);

    // ── Initialize STDIO serial at 9600 baud ──────────────────────────
    stdioSerialInit(9600);

    // ── Print startup banner ───────────────────────────────────────────
    printf("\r\n");
    printf("========================================\r\n");
    printf("  Lab 2.2 — Button Press Monitor        \r\n");
    printf("  FreeRTOS Preemptive Scheduler Demo    \r\n");
    printf("  Tasks: 3 | Tick: %u Hz               \r\n",
           (unsigned int)configTICK_RATE_HZ);
    printf("========================================\r\n");
    printf("GREEN  LED  = short press (< %u ms)\r\n",
           (unsigned int)SHORT_PRESS_THRESHOLD_MS);
    printf("RED    LED  = long press  (>= %u ms)\r\n",
           (unsigned int)SHORT_PRESS_THRESHOLD_MS);
    printf("YELLOW LED  = activity blink\r\n");
    printf("Report interval: 10 seconds\r\n");
    printf("Sync: binary semaphore + mutex\r\n");
    printf("========================================\r\n\r\n");

    // ── Create synchronization primitives ──────────────────────────────
    sharedStateInit();

    // ── Create FreeRTOS tasks ──────────────────────────────────────────
    xTaskCreate(
        vTaskMeasure,           // Task function
        "Measure",              // Human-readable name (for debugging)
        TASK_MEASURE_STACK,     // Stack size (bytes on AVR)
        NULL,                   // Parameters (unused)
        TASK_MEASURE_PRIORITY,  // Priority (3 = highest)
        NULL                    // Task handle (not needed)
    );

    xTaskCreate(
        vTaskStats,
        "Stats",
        TASK_STATS_STACK,
        NULL,
        TASK_STATS_PRIORITY,    // Priority (2 = medium)
        NULL
    );

    xTaskCreate(
        vTaskReport,
        "Report",
        TASK_REPORT_STACK,
        NULL,
        TASK_REPORT_PRIORITY,   // Priority (1 = lowest)
        NULL
    );

    // The FreeRTOS scheduler starts automatically after setup() returns
    // (handled by the Arduino_FreeRTOS library integration).
}

void lab2_2Loop() {

    // All application logic runs inside the three FreeRTOS tasks.
    // This function intentionally does nothing.
}
