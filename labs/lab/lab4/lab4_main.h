/**
 * @file lab4_main.h
 * @brief Lab 4 Entry Point Interface — Dual Actuator Control System
 *
 * Declares setup and loop for Laboratory Work 4:
 * "Dual Actuator Control — Binary Relay + Analog PWM with
 *  Keypad Interface, Signal Conditioning and FreeRTOS"
 *
 * Varianta C (100%): two actuators (binary + analog), both controlled
 * via a 4x4 keypad, with signal conditioning pipeline, overload
 * alerting, and LCD + serial reporting.
 *
 * Three FreeRTOS tasks:
 *   Task 1 — Input (50ms, prio 3): keypad scanning & command parsing
 *   Task 2 — Control (100ms, prio 2): debounce, conditioning, hardware
 *   Task 3 — Display (500ms, prio 1): LCD + structured serial reports
 *
 * Hardware (Arduino Mega 2560):
 *   D3  = PWM analog actuator (LED simulating motor)
 *   D7  = Relay control (active-HIGH)
 *   D8  = Green status LED
 *   D9  = Red alert LED
 *   D22-D25 = Keypad rows
 *   D26-D29 = Keypad columns
 *   SDA/SCL = LCD 1602 I2C
 */

#ifndef LAB4_MAIN_H
#define LAB4_MAIN_H

/** @brief Initialize hardware, create FreeRTOS tasks. */
void lab4Setup();

/** @brief Main loop — idle, all logic runs in FreeRTOS tasks. */
void lab4Loop();

#endif // LAB4_MAIN_H
