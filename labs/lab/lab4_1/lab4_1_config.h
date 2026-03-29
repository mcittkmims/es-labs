/**
 * @file lab4_1_config.h
 * @brief Lab 4.1 Pin Mapping and Configuration Constants
 *
 * Centralizes all hardware pin assignments and timing parameters
 * for the dual actuator control system. This single source of truth
 * ensures consistency between code, Wokwi simulation, and report.
 */

#ifndef LAB4_1_CONFIG_H
#define LAB4_1_CONFIG_H

// ── Pin Mapping ─────────────────────────────────────────────────────────
// Binary actuator: relay module on digital pin
#define PIN_RELAY           7

// Analog actuator: PWM output (motor/LED)
#define PIN_PWM_ACTUATOR    9

// Status LEDs
#define PIN_LED_BINARY     12   // Green — indicates relay state
#define PIN_LED_ANALOG     11   // Yellow — brightness tracks PWM duty

// LCD I2C
#define LCD_I2C_ADDR     0x27
#define LCD_COLS           16
#define LCD_ROWS            2

// Serial
#define SERIAL_BAUD      9600

// ── Signal Conditioning Parameters ──────────────────────────────────────
// Binary conditioner: debounce confirmations
#define BINARY_DEBOUNCE_COUNT   5

// Analog conditioner (SignalConditioner pipeline)
#define ANALOG_MEDIAN_WINDOW    5
#define ANALOG_EWMA_ALPHA     0.3f
#define ANALOG_MIN_CLAMP      0.0f
#define ANALOG_MAX_CLAMP    100.0f

// Ramp rate: max change in duty cycle per conditioning cycle (%)
#define ANALOG_RAMP_RATE      5.0f

// ── Task Timing (ms) ───────────────────────────────────────────────────
#define TASK_COMMAND_PERIOD_MS     50
#define TASK_BINARY_PERIOD_MS     100
#define TASK_ANALOG_PERIOD_MS     100
#define TASK_DISPLAY_PERIOD_MS    500

// ── FreeRTOS Stack Sizes ────────────────────────────────────────────────
#define STACK_COMMAND    192
#define STACK_BINARY     128
#define STACK_ANALOG     128
#define STACK_DISPLAY    256

#endif // LAB4_1_CONFIG_H
