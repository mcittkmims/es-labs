/**
 * @file lab1_2_main.h
 * @brief Lab 1.2 Entry Point Interface
 *
 * Declares the setup and loop functions for Laboratory Work 1.2:
 * "User Interaction: LCD + Keypad - Electronic Lock System".
 *
 * This lab implements a menu-driven keypad interface with LCD display
 * for controlling an electronic lock system. The system supports
 * locking, unlocking with password, password changes, and status
 * queries, all driven by a finite state machine (FSM).
 */

#ifndef LAB1_2_MAIN_H
#define LAB1_2_MAIN_H

/**
 * @brief Initialize all peripherals and the lock FSM.
 *
 * Configures STDIO serial (for debug logging), the I2C LCD display,
 * the 4x4 matrix keypad, the red and green LED indicators, and
 * initializes the lock FSM to its default state (locked, pwd=1234).
 */
void lab1_2Setup();

/**
 * @brief Main application loop.
 *
 * Reads keypad input (non-blocking), feeds keypresses to the FSM,
 * updates the LCD display when content changes, and controls LEDs
 * to reflect the current lock state (red=locked, green=unlocked).
 */
void lab1_2Loop();

#endif // LAB1_2_MAIN_H
