/**
 * @file task_input.h
 * @brief Lab 4 — Keypad Input Task Interface
 *
 * FreeRTOS task that scans the 4x4 keypad at 50ms intervals and
 * interprets user commands for both actuators:
 *
 *   A = Toggle relay ON/OFF (binary actuator)
 *   B = Switch to analog input mode
 *   0-9 = Enter PWM percentage digits (in analog mode)
 *   # = Confirm analog value
 *   * = Cancel / clear input
 *   C = Emergency stop (all actuators off)
 *   D = Show status report on serial
 */

#ifndef TASK_INPUT_H
#define TASK_INPUT_H

#include <Arduino_FreeRTOS.h>

/** @brief FreeRTOS task function for keypad input processing. */
void vTaskInput(void *pvParameters);

#endif // TASK_INPUT_H
