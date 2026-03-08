/**
 * @file task_conditioning.h
 * @brief Lab 3.1 — Signal Conditioning & Alert Task Interface (Task 2)
 *
 * Declares the FreeRTOS task function for threshold-based signal
 * conditioning and alert management. This task wakes on new sensor
 * data (via semaphore), applies hysteresis-based threshold detection
 * with debounce filtering, and controls the alert LEDs.
 *
 * Data flow:
 *   semaphore take → g_sensorData (mutex read) → ThresholdAlert FSM
 *   → g_alertData (mutex write) → LED control
 *
 * Task characteristics:
 *   Trigger:  Binary semaphore from Task 1
 *   Priority: 2 (medium — responds to acquisition events)
 *   Stack:    256 bytes
 */

#ifndef TASK_CONDITIONING_H
#define TASK_CONDITIONING_H

#include <Arduino_FreeRTOS.h>

/**
 * @brief FreeRTOS task function for signal conditioning and alerting.
 *
 * Waits on the binary semaphore from Task 1, reads sensor data,
 * applies threshold detection with hysteresis and debounce, updates
 * alert status, and drives the LED indicators.
 *
 * @param pvParameters Unused (NULL).
 */
void vTaskConditioning(void *pvParameters);

#endif // TASK_CONDITIONING_H
