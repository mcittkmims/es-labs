/**
 * @file lab5_2_config.cpp
 * @brief Lab 5.2 writable configuration arrays and PID presets.
 */

#include "lab5_2_config.h"

byte KEYPAD_ROW_PINS[4] = {11, 10, 9, 8};
byte KEYPAD_COL_PINS[4] = {7, 6, 5, 4};

const PidPreset PID_PRESETS[PID_PRESET_COUNT] = {
    {"SOFT", 8.0f, 0.10f, 1.0f},
    {"NORM", PID_DEFAULT_KP, PID_DEFAULT_KI, PID_DEFAULT_KD},
    {"FAST", 18.0f, 0.30f, 3.0f}
};
