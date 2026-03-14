/**
 * @file task_conditioning.h
 * @brief Lab 3.2 — Signal Conditioning & Alert Task Interface (Task 2)
 *
 * Declares the FreeRTOS task function for signal conditioning and alert
 * management. This task wakes on new sensor data (via semaphore), applies
 * a three-stage conditioning pipeline (saturation → median → EWMA), then
 * feeds the conditioned values into the threshold alert FSM.
 *
 * Data flow:
 *   semaphore take → g_sensorData raw (mutex read) → SignalConditioner
 *   → ThresholdAlert FSM → g_sensorData conditioned + g_alertData (mutex write)
 *   → LED control
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
 * Waits on the binary semaphore from Task 1, reads raw sensor data,
 * applies the SignalConditioner pipeline (saturation, median filter,
 * EWMA) to each sensor independently, feeds the conditioned values
 * into the ThresholdAlert FSM, updates alert status, and drives
 * the LED indicators.
 *
 * @param pvParameters Unused (NULL).
 */
void vTaskConditioning(void *pvParameters);

#endif // TASK_CONDITIONING_H
