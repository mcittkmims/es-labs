/**
 * @file lab4_1_main.h
 * @brief Lab 4.1 Entry Point — Dual Actuator Control System (Varianta C)
 *
 * Binary actuator (relay) + Analog actuator (PWM motor) controlled
 * via serial commands, with signal conditioning and LCD reporting.
 * Uses FreeRTOS for multi-task scheduling.
 */

#ifndef LAB4_1_MAIN_H
#define LAB4_1_MAIN_H

/** @brief Initialize all hardware and create FreeRTOS tasks. */
void lab4_1Setup();

/** @brief Main loop (empty — FreeRTOS scheduler runs tasks). */
void lab4_1Loop();

#endif // LAB4_1_MAIN_H
