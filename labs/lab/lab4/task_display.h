/**
 * @file task_display.h
 * @brief Lab 4 — Display & Reporting Task Interface
 *
 * FreeRTOS task running at 500ms that updates the LCD with actuator
 * status and periodically prints a structured serial report.
 */

#ifndef TASK_DISPLAY_H
#define TASK_DISPLAY_H

#include <Arduino_FreeRTOS.h>

/** @brief FreeRTOS task function for LCD display and serial reporting. */
void vTaskDisplay(void *pvParameters);

#endif // TASK_DISPLAY_H
