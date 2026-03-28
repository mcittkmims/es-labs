/**
 * @file task_input.h
 * @brief Input Reader Task Interface — Lab 4.1
 *
 * Declares the FreeRTOS task function that polls the hardware serial port
 * for user input. Characters are accumulated into a line buffer; upon
 * receiving a newline the buffer is parsed for an ON or OFF command and
 * the result is stored in the shared state under mutex protection.
 *
 * Recognized commands (case-insensitive, whitespace-tolerant):
 *   "ON"  → ActuatorControl::State::ON
 *   "OFF" → ActuatorControl::State::OFF
 *
 * Period:   TASK_INPUT_PERIOD_MS (10 ms polling interval)
 * Priority: TASK_INPUT_PRIORITY (highest — ensures responsive input)
 */

#ifndef TASK_INPUT_H
#define TASK_INPUT_H

#include <Arduino_FreeRTOS.h>

/**
 * @brief FreeRTOS input reader task.
 * @param params Unused (NULL).
 */
void vTaskInputReader(void *params);

#endif // TASK_INPUT_H
