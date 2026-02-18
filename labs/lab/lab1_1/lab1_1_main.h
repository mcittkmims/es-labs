/**
 * @file lab1_1_main.h
 * @brief Lab 1.1 Entry Point Interface
 *
 * Declares the setup and loop functions for Laboratory Work 1.1:
 * "User Interaction: STDIO - Serial Interface".
 *
 * This lab demonstrates serial communication using the STDIO library
 * (printf, fgets) to control an LED via text commands from a terminal.
 */

#ifndef LAB1_1_MAIN_H
#define LAB1_1_MAIN_H

/**
 * @brief Initialize all peripherals and display the welcome message.
 *
 * Configures the STDIO serial redirection, initializes the LED driver,
 * and prints a welcome banner with available commands to the terminal.
 */
void lab1_1Setup();

/**
 * @brief Main application loop — read commands and control the LED.
 *
 * Waits for a line of text from the serial terminal (blocking I/O),
 * parses the received command, executes the corresponding LED action,
 * and sends a confirmation or error message back to the terminal.
 */
void lab1_1Loop();

#endif // LAB1_1_MAIN_H
