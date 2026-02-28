/**
 * @file task_stats.cpp
 * @brief Lab 2.2 — Task 2 Implementation: Statistics and Blink Sequencer
 *
 * Implements the medium-priority FreeRTOS task that consumes press events
 * from Task 1, updates cumulative statistics, and drives the yellow LED
 * blink sequence.
 *
 * This task demonstrates two key FreeRTOS synchronization mechanisms:
 *   - Binary semaphore (event signaling): Task 2 blocks on
 *     xSemaphoreTake(xNewPressSemaphore, ...) until Task 1 gives the
 *     semaphore after detecting a complete press cycle.
 *   - Mutex (shared resource protection): Before accessing g_pressInfo
 *     and g_stats, Task 2 acquires xSharedDataMutex. This prevents data
 *     corruption if Task 3 simultaneously reads the statistics.
 *
 * The blink sequence is implemented as a simple blocking loop with
 * vTaskDelay(). This is dramatically simpler than the non-blocking
 * state-machine approach required in the bare-metal Lab 2.1 version,
 * showcasing a core advantage of preemptive multitasking: each task
 * can use blocking delays without affecting other tasks.
 */

#include "task_stats.h"
#include "shared_state.h"

#include <Arduino.h>
#include <Arduino_FreeRTOS.h>
#include <semphr.h>

// ──────────────────────────────────────────────────────────────────────────
// Task 2 implementation
// ──────────────────────────────────────────────────────────────────────────

void vTaskStats(void *pvParameters) {
    (void)pvParameters;  // Unused

    PressInfo_t localInfo;
    uint8_t     blinkCount;

    for (;;) {
        // ── Block until Task 1 signals a new press event ───────────────
        // The binary semaphore acts as an event flag: Task 2 sleeps here
        // with zero CPU usage until Task 1 gives the semaphore.
        xSemaphoreTake(xNewPressSemaphore, portMAX_DELAY);

        // ── Read press info and update statistics under mutex ──────────
        if (xSemaphoreTake(xSharedDataMutex, portMAX_DELAY) == pdTRUE) {
            // Copy press info locally so we can release the mutex quickly.
            localInfo = g_pressInfo;

            // Update cumulative statistics.
            g_stats.totalPresses++;
            g_stats.totalDurationMs += localInfo.duration;

            if (localInfo.isShort) {
                g_stats.shortPresses++;
            } else {
                g_stats.longPresses++;
            }

            xSemaphoreGive(xSharedDataMutex);
        }

        // ── Yellow LED blink sequence ──────────────────────────────────
        // 5 blinks for short press, 10 blinks for long press.
        // Each blink = LED ON for BLINK_HALF_PERIOD_MS, then OFF for
        // BLINK_HALF_PERIOD_MS. Using vTaskDelay() releases the CPU to
        // other tasks during the wait — this is a key advantage over
        // the bare-metal polling approach.
        blinkCount = localInfo.isShort ? BLINK_COUNT_SHORT : BLINK_COUNT_LONG;

        for (uint8_t i = 0; i < blinkCount; i++) {
            digitalWrite(PIN_LED_YELLOW, HIGH);
            vTaskDelay(pdMS_TO_TICKS(BLINK_HALF_PERIOD_MS));

            digitalWrite(PIN_LED_YELLOW, LOW);
            vTaskDelay(pdMS_TO_TICKS(BLINK_HALF_PERIOD_MS));
        }
    }
}
