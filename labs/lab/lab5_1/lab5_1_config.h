/**
 * @file lab5_1_config.h
 * @brief Lab 5.1 configuration and pin mapping.
 */

#ifndef LAB5_1_CONFIG_H
#define LAB5_1_CONFIG_H

#include <Arduino.h>
#include <Arduino_FreeRTOS.h>

// Hardware pin mapping from the provided Arduino Mega wiring.
static const uint8_t PIN_DHT22 = 2;
static const uint8_t PIN_RELAY = 12;
static const uint8_t PIN_SETPOINT_POT = A0;

extern byte KEYPAD_ROW_PINS[4];
extern byte KEYPAD_COL_PINS[4];

static const uint8_t LCD_I2C_ADDRESS = 0x27;
static const uint8_t LCD_COLS = 16;
static const uint8_t LCD_ROWS = 2;

// Many real relay modules are active-low. The supplied Wokwi relay module
// expects active-high control; change this constant if the physical relay
// used during the lab energizes on LOW.
static const bool RELAY_ACTIVE_HIGH = true;

// Control parameters.
static const float SETPOINT_MIN_C = 15.0f;
static const float SETPOINT_MAX_C = 35.0f;
static const float SETPOINT_DEFAULT_C = 25.0f;
static const float SETPOINT_STEP_C = 0.5f;

static const float HYSTERESIS_MIN_C = 0.5f;
static const float HYSTERESIS_MAX_C = 5.0f;
static const float HYSTERESIS_DEFAULT_C = 2.0f;
static const float HYSTERESIS_STEP_C = 0.5f;

static const uint8_t SETPOINT_INPUT_MAX_DIGITS = 3;

// FreeRTOS task periods.
static const uint16_t TASK_INPUT_PERIOD_MS = 50;
static const uint16_t TASK_ACQUISITION_PERIOD_MS = 2000;
static const uint16_t TASK_DISPLAY_PERIOD_MS = 500;

// FreeRTOS task stack sizes and priorities.
static const configSTACK_DEPTH_TYPE TASK_INPUT_STACK = 256;
static const configSTACK_DEPTH_TYPE TASK_ACQUISITION_STACK = 384;
static const configSTACK_DEPTH_TYPE TASK_CONTROL_STACK = 320;
static const configSTACK_DEPTH_TYPE TASK_ACTUATION_STACK = 256;
static const configSTACK_DEPTH_TYPE TASK_DISPLAY_STACK = 640;

static const UBaseType_t TASK_INPUT_PRIORITY = 3;
static const UBaseType_t TASK_ACQUISITION_PRIORITY = 3;
static const UBaseType_t TASK_CONTROL_PRIORITY = 2;
static const UBaseType_t TASK_ACTUATION_PRIORITY = 2;
static const UBaseType_t TASK_DISPLAY_PRIORITY = 1;

#endif // LAB5_1_CONFIG_H
