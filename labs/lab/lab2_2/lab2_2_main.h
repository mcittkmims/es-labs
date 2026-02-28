/**
 * @file lab2_2_main.h
 * @brief Lab 2.2 Entry Point Interface
 *
 * Declares the setup and loop functions for Laboratory Work 2.2:
 * "Button Press Duration Monitoring with Visual Signaling and Periodic
 *  Reporting using FreeRTOS Preemptive Scheduling".
 *
 * This lab ports the Lab 2.1 bare-metal cooperative application to FreeRTOS,
 * demonstrating preemptive multitasking with binary semaphores for event
 * signaling and mutexes for protecting shared variables.
 *
 * Three FreeRTOS tasks implement the same functionality as Lab 2.1:
 *
 *   Task 1 — Button Detection & LED Signaling (10 ms period, priority 3):
 *     Monitors a push button using a debounce FSM, measures press duration,
 *     and signals the result via green/red LEDs and a binary semaphore.
 *
 *   Task 2 — Statistics & Yellow LED Blink (event-driven, priority 2):
 *     Waits on the binary semaphore, updates press counters under mutex
 *     protection, and drives a blocking yellow LED blink sequence.
 *
 *   Task 3 — Periodic STDIO Reporting (10 s period, priority 1):
 *     Uses vTaskDelayUntil() for drift-free 10-second reporting, reads
 *     and resets statistics under mutex protection.
 *
 * Hardware pin mapping (Arduino Mega 2560):
 *   PIN_BUTTON     = 7  (active-LOW with internal pull-up)
 *   PIN_LED_GREEN  = 8  (short press indicator)
 *   PIN_LED_RED    = 9  (long press indicator)
 *   PIN_LED_YELLOW = 10 (activity blink)
 */

#ifndef LAB2_2_MAIN_H
#define LAB2_2_MAIN_H

/**
 * @brief Initialize hardware, create synchronization primitives and FreeRTOS tasks.
 *
 * Configures GPIO pins, initializes the STDIO serial interface, creates
 * the binary semaphore and mutex, and spawns three FreeRTOS tasks. The
 * FreeRTOS scheduler starts automatically after setup() returns.
 */
void lab2_2Setup();

/**
 * @brief Main loop — idle context when all FreeRTOS tasks are blocked.
 *
 * All application work is performed inside FreeRTOS tasks. The loop()
 * function yields CPU time via vTaskDelay() to allow the idle task to
 * run and minimise power consumption.
 */
void lab2_2Loop();

#endif // LAB2_2_MAIN_H
