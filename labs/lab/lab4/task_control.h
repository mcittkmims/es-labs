/**
 * @file task_control.h
 * @brief Lab 4 — Actuator Control & Conditioning Task Interface
 *
 * FreeRTOS task running at 100ms that:
 *   - Applies debouncing to binary relay commands
 *   - Processes analog commands through the conditioning pipeline
 *     (saturation, median filter, EWMA, ramping)
 *   - Controls relay and PWM hardware outputs
 *   - Evaluates overload threshold alerts
 */

#ifndef TASK_CONTROL_H
#define TASK_CONTROL_H

#include <Arduino_FreeRTOS.h>

/** @brief FreeRTOS task function for actuator control and conditioning. */
void vTaskControl(void *pvParameters);

#endif // TASK_CONTROL_H
