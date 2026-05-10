/**
 * @file lab6_1_main.h
 * @brief Lab 6.1 Entry Point Interface
 *
 * Declares the setup and loop functions for Laboratory Work 6.1:
 * "Behavioural control with Finite State Machines --- Button-LED".
 *
 * The lab implements a two-state Moore automaton that toggles an LED
 * on every confirmed (debounced) press of a push button. The current
 * state of the FSM is reported in real time over the STDIO serial
 * interface.
 */

#ifndef LAB6_1_MAIN_H
#define LAB6_1_MAIN_H

/**
 * @brief Initialize peripherals, FSM, and serial reporting.
 *
 * Brings up the STDIO serial bridge, configures the Button driver
 * (debounced GPIO input), the Led driver (digital output), and the
 * ButtonLedFsm Moore automaton. Prints a welcome banner that lists
 * the active wiring and the FSM state table.
 */
void lab6_1Setup();

/**
 * @brief Periodic application step: drive button -> FSM -> LED -> STDIO.
 *
 * Each iteration polls the debounced Button driver, forwards confirmed
 * press edges to the FSM, applies the FSM's Moore output to the LED,
 * and prints a state-transition line to the serial terminal whenever
 * the state actually changes.
 */
void lab6_1Loop();

#endif // LAB6_1_MAIN_H
