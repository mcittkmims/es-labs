/**
 * @file lab2_1_main.h
 * @brief Lab 2.1 Entry Point Interface
 *
 * Declares the setup and loop functions for Laboratory Work 2.1:
 * "Button Press Duration Monitoring with Visual Signaling and Periodic Reporting
 *  using a Non-Preemptive Bare-Metal Task Scheduler".
 *
 * This lab implements three cooperative tasks driven by the TaskScheduler library:
 *
 *   Task 1 — Button Detection & LED Signaling (10 ms period):
 *     Monitors a push button using a debounce state machine, measures the
 *     duration of each press, and signals the result via a green LED (short
 *     press, < 500 ms) or red LED (long press, ≥ 500 ms).
 *
 *   Task 2 — Statistics & Yellow LED Blink (50 ms period):
 *     Consumes new-press events produced by Task 1, updates press counters and
 *     duration accumulators, and drives a yellow LED blink sequence: 5 rapid
 *     blinks for a short press, 10 rapid blinks for a long press.
 *
 *   Task 3 — Periodic STDIO Reporting (10 000 ms period):
 *     Every 10 seconds, prints a statistics report (total presses, short/long
 *     counts, average duration) over the serial STDIO interface, then resets
 *     all accumulators for the next measurement window.
 *
 * Hardware pin mapping (Arduino Mega 2560):
 *   PIN_BUTTON     = 7  (active-LOW with internal pull-up)
 *   PIN_LED_GREEN  = 8  (short press indicator)
 *   PIN_LED_RED    = 9  (long press indicator)
 *   PIN_LED_YELLOW = 10 (activity blink)
 */

#ifndef LAB2_1_MAIN_H
#define LAB2_1_MAIN_H

/**
 * @brief Initialize peripherals and the task scheduler for Lab 2.1.
 *
 * Configures the button pin (INPUT_PULLUP), three LED output pins, and the
 * STDIO serial interface at 9600 baud. Registers the three tasks in the
 * TaskScheduler and prints a startup banner to the serial terminal.
 */
void lab2_1Setup();

/**
 * @brief Main application loop — drives the non-preemptive task scheduler.
 *
 * Calls schedulerRun() on every iteration. The scheduler evaluates which task
 * is due according to its period and offset, then executes exactly one task
 * per call, ensuring non-preemptive, single-task-per-tick scheduling.
 */
void lab2_1Loop();

#endif // LAB2_1_MAIN_H
