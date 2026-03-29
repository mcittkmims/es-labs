/**
 * @file lab4_config.h
 * @brief Lab 4 — Shared Configuration and Pin Mapping
 *
 * Centralizes all pin assignments, timing constants, conditioning
 * parameters, and FreeRTOS task configuration for the dual-actuator
 * control system (binary relay + analog PWM).
 *
 * Pin mapping (Arduino Mega 2560):
 *   D3  = PWM analog actuator output (LED simulating motor)
 *   D7  = Relay control pin (active-HIGH, drives relay module)
 *   D8  = Status LED green (system OK)
 *   D9  = Status LED red (alert / overload)
 *   D22-D25 = Keypad rows (R1-R4)
 *   D26-D29 = Keypad columns (C1-C4)
 *   SDA(20) = LCD I2C data
 *   SCL(21) = LCD I2C clock
 */

#ifndef LAB4_CONFIG_H
#define LAB4_CONFIG_H

#include <Arduino.h>

// ── Pin assignments ─────────────────────────────────────────────────
static const uint8_t PIN_RELAY       = 7;   // Binary actuator relay
static const uint8_t PIN_PWM_ACT     = 3;   // Analog actuator (PWM)
static const uint8_t PIN_LED_GREEN   = 8;   // System normal indicator
static const uint8_t PIN_LED_RED     = 9;   // Alert indicator

// Keypad pins (4x4 matrix)
static byte KEYPAD_ROW_PINS[4] = {22, 23, 24, 25};
static byte KEYPAD_COL_PINS[4] = {26, 27, 28, 29};

// LCD I2C
static const uint8_t LCD_I2C_ADDR = 0x27;
static const uint8_t LCD_COLS     = 16;
static const uint8_t LCD_ROWS     = 2;

// ── Timing (milliseconds) ──────────────────────────────────────────
static const uint16_t TASK_INPUT_PERIOD_MS      = 50;   // Keypad scan
static const uint16_t TASK_CONTROL_PERIOD_MS    = 100;  // Actuator control
static const uint16_t TASK_DISPLAY_PERIOD_MS    = 500;  // LCD + serial

// ── FreeRTOS task configuration ────────────────────────────────────
static const uint16_t TASK_INPUT_STACK      = 256;
static const uint16_t TASK_CONTROL_STACK    = 256;
static const uint16_t TASK_DISPLAY_STACK    = 512;
static const uint8_t TASK_INPUT_PRIORITY   = 3;
static const uint8_t TASK_CONTROL_PRIORITY = 2;
static const uint8_t TASK_DISPLAY_PRIORITY = 1;

// ── Binary actuator debounce ────────────────────────────────────────
static const uint8_t RELAY_DEBOUNCE_COUNT = 3; // Consecutive confirmations

// ── Analog actuator conditioning parameters ─────────────────────────
static const uint8_t  ACT_MEDIAN_WINDOW = 5;
static const float    ACT_EWMA_ALPHA    = 0.4f;
static const float    ACT_MIN_CLAMP     = 0.0f;
static const float    ACT_MAX_CLAMP     = 100.0f;
static const float    ACT_RAMP_STEP     = 5.0f;  // Max % change per cycle

// ── Alert thresholds for analog actuator ────────────────────────────
static const float    OVERLOAD_THRESHOLD_HIGH = 90.0f;
static const float    OVERLOAD_THRESHOLD_LOW  = 80.0f;
static const uint8_t  OVERLOAD_DEBOUNCE_COUNT = 3;

#endif // LAB4_CONFIG_H
