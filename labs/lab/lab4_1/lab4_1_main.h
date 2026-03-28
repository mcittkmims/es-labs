/**
 * @file lab4_1_main.h
 * @brief Lab 4.1 Entry Point Interface
 *
 * Declares the setup and loop functions for Laboratory Work 4.1:
 * "Control Actuator Binar — Binary Actuator Control with Signal Conditioning"
 *
 * This lab implements a binary actuator control system that:
 *   - Receives ON/OFF commands from the user via STDIO serial (9600 baud)
 *   - Applies a three-stage signal conditioning pipeline to each command:
 *       1. Saturation    — coerces input to strict {OFF, ON}
 *       2. Debouncing    — requires stable input for DEBOUNCE_PERIOD_MS
 *       3. Persistence   — requires PERSIST_COUNT consecutive confirmations
 *   - Drives a binary actuator (LED on D7, simulating relay output)
 *   - Reports state and conditioning status on a 16x2 I2C LCD (500 ms)
 *
 * Three FreeRTOS tasks implement the pipeline:
 *
 *   Task 1 — Input Reader (10 ms polling, priority 3):
 *     Polls the serial port, accumulates characters, parses "ON" / "OFF"
 *     commands, and writes results to g_actuator.pendingCmd under mutex.
 *
 *   Task 2 — Actuator Control (75 ms period, priority 2):
 *     Reads pendingCmd, runs ActuatorControl::update() (BinaryConditioner
 *     pipeline), drives the LED GPIO, and updates shared actuator state.
 *
 *   Task 3 — Display & Reporting (500 ms period, priority 1):
 *     Updates the I2C LCD with actuator state and conditioning status;
 *     prints a structured STDIO report every 2 seconds.
 *
 * Hardware pin mapping (Arduino Mega 2560):
 *   D7  = LED actuator output (simulates relay load)
 *   D20 = LCD 1602 I2C SDA
 *   D21 = LCD 1602 I2C SCL
 *   USB = STDIO serial terminal at 9600 baud
 */

#ifndef LAB4_1_MAIN_H
#define LAB4_1_MAIN_H

/**
 * @brief Initialize hardware, shared state, and FreeRTOS tasks.
 *
 * Configures STDIO serial, prints the startup banner, initializes the
 * shared actuator state and mutex, then creates and starts the three
 * FreeRTOS tasks. The FreeRTOS scheduler starts automatically after
 * setup() returns (Arduino_FreeRTOS library integration).
 */
void lab4_1Setup();

/**
 * @brief Main loop — idle while all FreeRTOS tasks are running.
 *
 * All application logic executes within the three FreeRTOS tasks.
 * This function intentionally does nothing.
 */
void lab4_1Loop();

#endif // LAB4_1_MAIN_H
