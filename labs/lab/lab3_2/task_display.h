/**
 * @file task_display.h
 * @brief Lab 3.2 — Display & Reporting Task Interface (Task 3)
 *
 * Declares the FreeRTOS task function for periodic display update and
 * structured STDIO reporting. This task runs at 500 ms intervals and
 * presents temperature readings (raw and conditioned), alert states,
 * conditioning configuration, and system statistics on both the LCD
 * display and the serial terminal.
 *
 * Data flow:
 *   g_sensorData + g_alertData (mutex read) → LCD display + STDIO printf
 *
 * Task characteristics:
 *   Period:   500 ms (configurable via TASK_DISPLAY_PERIOD_MS)
 *   Priority: 1 (lowest — display can tolerate jitter)
 *   Stack:    512 bytes (printf requires more stack)
 */

#ifndef TASK_DISPLAY_H
#define TASK_DISPLAY_H

#include <Arduino_FreeRTOS.h>

/**
 * @brief FreeRTOS task function for display and reporting.
 *
 * Periodically reads sensor data and alert status under mutex,
 * formats the data for the LCD (2 lines alternating), and prints
 * a structured report to STDIO via printf that includes all
 * conditioning pipeline intermediate values.
 *
 * LCD display alternates between:
 *   Page 0: "A:XX.XC D:XX.XC" / "System: OK" or alerts
 *   Page 1: "Med:5 Alpha:0.3" / "TH:30 TL:28"
 *
 * STDIO report includes raw values, median-filtered values, EWMA
 * values, conditioning configuration, alert states, and statistics.
 *
 * @param pvParameters Unused (NULL).
 */
void vTaskDisplay(void *pvParameters);

#endif // TASK_DISPLAY_H
