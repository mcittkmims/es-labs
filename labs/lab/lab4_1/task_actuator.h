/**
 * @file task_actuator.h
 * @brief Actuator Control Task Interface — Lab 4.1
 *
 * Declares the FreeRTOS task function responsible for running the signal
 * conditioning pipeline and updating the binary actuator hardware.
 *
 * On each tick the task:
 *   1. Reads the latest raw command from shared state.
 *   2. Calls ActuatorControl::update(), which internally runs:
 *         saturation → debounce → persistent validation (BinaryConditioner)
 *   3. Writes the conditioned state and conditioning metadata back to
 *      shared state for the display task to consume.
 *
 * Period:   TASK_ACTUATOR_PERIOD_MS (75 ms)
 * Priority: TASK_ACTUATOR_PRIORITY (medium)
 */

#ifndef TASK_ACTUATOR_H
#define TASK_ACTUATOR_H

#include <Arduino_FreeRTOS.h>

/**
 * @brief FreeRTOS actuator control task.
 * @param params Unused (NULL).
 */
void vTaskActuator(void *params);

#endif // TASK_ACTUATOR_H
