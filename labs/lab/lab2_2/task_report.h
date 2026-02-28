/**
 * @file task_report.h
 * @brief Lab 2.2 — Task 3: Periodic STDIO Statistics Report
 *
 * Declares the FreeRTOS task function for Task 3, which periodically
 * reads and reports cumulative button press statistics over the serial
 * STDIO interface.
 */

#ifndef TASK_REPORT_H
#define TASK_REPORT_H

/**
 * @brief FreeRTOS task function — Periodic statistics reporting.
 *
 * Runs with a strict 10-second period using vTaskDelayUntil() for
 * drift-free scheduling (equivalent to the bare-metal scheduler's
 * absolute nextRun advancement). On each iteration:
 *   1. Acquires xSharedDataMutex to atomically read and reset g_stats.
 *   2. Computes average press duration (integer division).
 *   3. Prints a formatted report to the serial STDIO terminal.
 *
 * The use of vTaskDelayUntil() (vs vTaskDelay()) ensures that the report
 * period remains exactly 10 seconds regardless of how long the printf()
 * call takes, preventing cumulative drift.
 *
 * @param pvParameters Unused (NULL).
 */
void vTaskReport(void *pvParameters);

#endif // TASK_REPORT_H
