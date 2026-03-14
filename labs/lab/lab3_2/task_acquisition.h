/**
 * @file task_acquisition.h
 * @brief Lab 3.2 — Sensor Acquisition Task Interface (Task 1)
 *
 * Declares the FreeRTOS task function for periodic sensor acquisition.
 * This task runs at 50 ms intervals (20 Hz) and reads both the analog
 * NTC thermistor (via ADC) and the digital DS18B20 sensor (via OneWire).
 *
 * Data flow:
 *   ADC / OneWire → sensor drivers → g_sensorData (mutex) → semaphore give
 *
 * Task characteristics:
 *   Period:   50 ms (configurable via TASK_ACQUISITION_PERIOD_MS)
 *   Priority: 3 (highest — sensor reading must not be starved)
 *   Stack:    256 bytes
 */

#ifndef TASK_ACQUISITION_H
#define TASK_ACQUISITION_H

#include <Arduino_FreeRTOS.h>

/**
 * @brief FreeRTOS task function for sensor acquisition.
 *
 * Periodically reads both temperature sensors, stores the raw and
 * converted values in g_sensorData under mutex protection, and gives
 * the binary semaphore to notify Task 2 (conditioning).
 *
 * @param pvParameters Unused (NULL).
 */
void vTaskAcquisition(void *pvParameters);

#endif // TASK_ACQUISITION_H
