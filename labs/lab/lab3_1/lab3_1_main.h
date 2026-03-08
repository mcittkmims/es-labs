/**
 * @file lab3_1_main.h
 * @brief Lab 3.1 Entry Point Interface
 *
 * Declares the setup and loop functions for Laboratory Work 3.1:
 * "Dual-Sensor Temperature Monitoring with Threshold Alerting
 *  using FreeRTOS Preemptive Scheduling".
 *
 * This lab implements a comprehensive temperature monitoring system
 * that reads from two sensors — one analog (NTC thermistor via ADC)
 * and one digital (DS18B20 via OneWire) — and applies hysteresis-based
 * threshold detection with debounce filtering for reliable alerting.
 *
 * Three FreeRTOS tasks implement the monitoring pipeline:
 *
 *   Task 1 — Sensor Acquisition (50 ms period, priority 3):
 *     Reads both temperature sensors at 20 Hz, converts raw readings
 *     to degrees Celsius, and stores results in shared memory with
 *     mutex protection and semaphore signaling.
 *
 *   Task 2 — Signal Conditioning & Alerting (event-driven, priority 2):
 *     Waits on the acquisition semaphore, applies hysteresis-based
 *     threshold detection for both sensors independently, tracks
 *     debounce state, and controls LED indicators.
 *
 *   Task 3 — Display & Reporting (500 ms period, priority 1):
 *     Updates the LCD display with current temperatures and alert
 *     status, and prints structured STDIO reports every 2 seconds.
 *
 * Hardware pin mapping (Arduino Mega 2560):
 *   A0  = NTC thermistor analog output (voltage divider with 10K series R)
 *   D2  = DS18B20 OneWire data pin (4.7K pull-up to VCC)
 *   D8  = Green LED (system normal indicator)
 *   D9  = Red LED (analog sensor alert)
 *   D10 = Yellow LED (digital sensor alert)
 *   SDA = LCD 1602 I2C data (pin 20 on Mega)
 *   SCL = LCD 1602 I2C clock (pin 21 on Mega)
 */

#ifndef LAB3_1_MAIN_H
#define LAB3_1_MAIN_H

/**
 * @brief Initialize hardware, create FreeRTOS tasks and start the scheduler.
 *
 * Configures STDIO serial, creates synchronization primitives (mutex
 * and binary semaphore), prints the startup banner, and spawns three
 * FreeRTOS tasks for sensor acquisition, conditioning, and display.
 */
void lab3_1Setup();

/**
 * @brief Main loop — idle when all FreeRTOS tasks are blocked.
 *
 * All application logic runs inside the three FreeRTOS tasks.
 * This function intentionally does nothing.
 */
void lab3_1Loop();

#endif // LAB3_1_MAIN_H
