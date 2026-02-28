/**
 * @file task_report.cpp
 * @brief Lab 2.2 — Task 3 Implementation: Periodic STDIO Report
 *
 * Implements the lowest-priority FreeRTOS task that produces a formatted
 * statistics report every 10 seconds. The task uses vTaskDelayUntil() for
 * precise, drift-free periodic execution — a direct FreeRTOS equivalent of
 * the bare-metal scheduler's deadline-relative advancement.
 *
 * The statistics (total presses, short/long counts, average duration) are
 * read and reset atomically under mutex protection. This ensures Task 2
 * cannot update g_stats while Task 3 is reading, preventing torn reads
 * and counter skew.
 */

#include "task_report.h"
#include "shared_state.h"

#include <Arduino.h>
#include <Arduino_FreeRTOS.h>
#include <semphr.h>
#include <stdio.h>

// ──────────────────────────────────────────────────────────────────────────
// Task 3 implementation
// ──────────────────────────────────────────────────────────────────────────

void vTaskReport(void *pvParameters) {
    (void)pvParameters;  // Unused

    // vTaskDelayUntil() requires the previous wake time to compute the
    // next absolute deadline. Initialised to the current tick count.
    TickType_t xLastWakeTime = xTaskGetTickCount();

    // Local snapshot of statistics — read under mutex, printed after release.
    Stats_t snapshot;

    for (;;) {
        // ── Wait until the next 10-second boundary ─────────────────────
        vTaskDelayUntil(&xLastWakeTime, pdMS_TO_TICKS(TASK_REPORT_PERIOD_MS));

        // ── Atomically read and reset statistics ───────────────────────
        if (xSemaphoreTake(xSharedDataMutex, portMAX_DELAY) == pdTRUE) {
            // Copy to local snapshot.
            snapshot = g_stats;

            // Reset accumulators for the next 10-second window.
            g_stats.totalPresses    = 0;
            g_stats.shortPresses    = 0;
            g_stats.longPresses     = 0;
            g_stats.totalDurationMs = 0;

            xSemaphoreGive(xSharedDataMutex);
        }

        // ── Compute average duration ───────────────────────────────────
        uint32_t avgMs = (snapshot.totalPresses > 0)
                         ? (snapshot.totalDurationMs / snapshot.totalPresses)
                         : 0;

        // ── Print formatted report ─────────────────────────────────────
        printf("\r\n===== [10s Report] =====\r\n");
        printf("Total presses    : %lu\r\n", (unsigned long)snapshot.totalPresses);
        printf("Short presses    : %lu  (< %u ms)\r\n",
               (unsigned long)snapshot.shortPresses,
               (unsigned int)SHORT_PRESS_THRESHOLD_MS);
        printf("Long presses     : %lu  (>= %u ms)\r\n",
               (unsigned long)snapshot.longPresses,
               (unsigned int)SHORT_PRESS_THRESHOLD_MS);
        printf("Average duration : %lu ms\r\n", (unsigned long)avgMs);
        printf("========================\r\n");
    }
}
