/**
 * @file task_display.h
 * @brief Display and Reporting Task Interface — Lab 4.1
 *
 * Declares the FreeRTOS task function that updates the LCD display and
 * generates structured STDIO reports at a low recurrence rate.
 *
 * LCD layout (16x2 I2C):
 *   Line 1: "Actuator: ON " or "Actuator: OFF"
 *   Line 2: "STABLE    " or "DEBOUNCING"
 *
 * A detailed STDIO report is printed every STDIO_REPORT_EVERY_N ticks
 * (approximately every 2 seconds), showing actuator state, raw command,
 * conditioning status, confirmation progress, and total toggle count.
 *
 * Period:   TASK_DISPLAY_PERIOD_MS (500 ms)
 * Priority: TASK_DISPLAY_PRIORITY (lowest)
 */

#ifndef TASK_DISPLAY_H
#define TASK_DISPLAY_H

#include <Arduino_FreeRTOS.h>

/**
 * @brief FreeRTOS display and reporting task.
 * @param params Unused (NULL).
 */
void vTaskDisplay(void *params);

#endif // TASK_DISPLAY_H
