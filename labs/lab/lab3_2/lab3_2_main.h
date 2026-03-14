/**
 * @file lab3_2_main.h
 * @brief Lab 3.2 Entry Point Interface
 *
 * Declares the setup and loop functions for Laboratory Work 3.2:
 * "Analog Signal Acquisition — Dual-Sensor Temperature Monitoring
 *  with Signal Conditioning Pipeline using FreeRTOS".
 *
 * This lab implements a temperature monitoring system that reads from
 * two sensors — one analog (NTC thermistor via ADC) and one digital
 * (DS18B20 via OneWire) — and applies a three-stage signal conditioning
 * pipeline (saturation, median filter, EWMA) before threshold alerting.
 *
 * Three FreeRTOS tasks implement the monitoring pipeline:
 *
 *   Task 1 — Sensor Acquisition (50 ms period, priority 3):
 *     Reads both temperature sensors at 20 Hz, converts raw readings
 *     to degrees Celsius, and stores results in shared memory with
 *     mutex protection and semaphore signaling.
 *
 *   Task 2 — Signal Conditioning & Alerting (event-driven, priority 2):
 *     Waits on the acquisition semaphore, applies the conditioning
 *     pipeline (saturate → median → EWMA) to each sensor independently,
 *     feeds conditioned values into hysteresis threshold detection,
 *     and controls LED indicators.
 *
 *   Task 3 — Display & Reporting (500 ms period, priority 1):
 *     Updates the LCD display with conditioned temperatures and alert
 *     status, and prints structured STDIO reports every 2 seconds
 *     showing all conditioning pipeline intermediate values.
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

#ifndef LAB3_2_MAIN_H
#define LAB3_2_MAIN_H

/**
 * @brief Initialize hardware, create FreeRTOS tasks and start the scheduler.
 *
 * Configures STDIO serial, creates synchronization primitives (mutex
 * and binary semaphore), prints the startup banner with conditioning
 * configuration, and spawns three FreeRTOS tasks for sensor acquisition,
 * conditioning, and display.
 */
void lab3_2Setup();

/**
 * @brief Main loop — idle when all FreeRTOS tasks are blocked.
 *
 * All application logic runs inside the three FreeRTOS tasks.
 * This function intentionally does nothing.
 */
void lab3_2Loop();

#endif // LAB3_2_MAIN_H
