/**
 * @file lab5_2_config.h
 * @brief Lab 5.2 configuration and pin mapping.
 */

#ifndef LAB5_2_CONFIG_H
#define LAB5_2_CONFIG_H

#include <Arduino.h>
#include <Arduino_FreeRTOS.h>
#include <DHT.h>

static const uint8_t PIN_DHT_SENSOR = 2;
static const uint8_t DHT_SENSOR_TYPE = DHT11;
static const uint8_t PIN_SETPOINT_POT = A0;

// Reference physical fan branch: L293D/L298-style H-bridge.
static const uint8_t PIN_FAN_PWM = 3;
static const uint8_t PIN_FAN_IN1 = 16;
static const uint8_t PIN_FAN_IN2 = 17;

extern byte KEYPAD_ROW_PINS[4];
extern byte KEYPAD_COL_PINS[4];

static const uint8_t LCD_I2C_ADDRESS = 0x27;
static const uint8_t LCD_COLS = 16;
static const uint8_t LCD_ROWS = 2;

static const float SETPOINT_MIN_C = 18.0f;
static const float SETPOINT_MAX_C = 35.0f;
static const float SETPOINT_DEFAULT_C = 25.0f;
static const float SETPOINT_STEP_C = 0.5f;
static const uint8_t SETPOINT_INPUT_MAX_DIGITS = 3;

static const float PID_OUTPUT_MIN_PERCENT = 0.0f;
static const float PID_OUTPUT_MAX_PERCENT = 100.0f;
static const float FAN_MIN_RUNNING_DUTY_PERCENT = 30.0f;
static const float FAN_STOP_THRESHOLD_PERCENT = 1.0f;

static const float PID_DEFAULT_KP = 12.0f;
static const float PID_DEFAULT_KI = 0.18f;
static const float PID_DEFAULT_KD = 2.0f;

struct PidPreset {
    const char *name;
    float kp;
    float ki;
    float kd;
};

static const uint8_t PID_PRESET_COUNT = 3;
extern const PidPreset PID_PRESETS[PID_PRESET_COUNT];

static const uint16_t TASK_INPUT_PERIOD_MS = 50;
static const uint16_t TASK_ACQUISITION_PERIOD_MS = 2000;
static const uint16_t TASK_DISPLAY_PERIOD_MS = 500;

static const configSTACK_DEPTH_TYPE TASK_INPUT_STACK = 256;
static const configSTACK_DEPTH_TYPE TASK_ACQUISITION_STACK = 384;
static const configSTACK_DEPTH_TYPE TASK_CONTROL_STACK = 384;
static const configSTACK_DEPTH_TYPE TASK_ACTUATION_STACK = 256;
static const configSTACK_DEPTH_TYPE TASK_DISPLAY_STACK = 704;

static const UBaseType_t TASK_INPUT_PRIORITY = 3;
static const UBaseType_t TASK_ACQUISITION_PRIORITY = 3;
static const UBaseType_t TASK_CONTROL_PRIORITY = 2;
static const UBaseType_t TASK_ACTUATION_PRIORITY = 2;
static const UBaseType_t TASK_DISPLAY_PRIORITY = 1;

#endif // LAB5_2_CONFIG_H
