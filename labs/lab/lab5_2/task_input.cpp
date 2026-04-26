/**
 * @file task_input.cpp
 * @brief Lab 5.2 keypad input task implementation.
 */

#include "task_input.h"
#include "lab5_2_config.h"
#include "shared_state.h"
#include "KeypadInput.h"

#include <Arduino_FreeRTOS.h>
#include <stdlib.h>
#include <stdio.h>

static KeypadInput s_keypad(KEYPAD_ROW_PINS, KEYPAD_COL_PINS);

static float clampFloat(float value, float minValue, float maxValue) {
    if (value < minValue) return minValue;
    if (value > maxValue) return maxValue;
    return value;
}

static void enterManualMode(Lab5PidState *state) {
    if (state->setpointSource == SETPOINT_SOURCE_POT) {
        state->manualSetpointC = state->activeSetpointC;
    }
    state->setpointSource = SETPOINT_SOURCE_MANUAL;
    state->activeSetpointC = state->manualSetpointC;
}

static void applyPidPreset(Lab5PidState *state, uint8_t presetIndex) {
    if (presetIndex >= PID_PRESET_COUNT) {
        presetIndex = 0;
    }

    state->pidPresetIndex = presetIndex;
    state->kp = PID_PRESETS[presetIndex].kp;
    state->ki = PID_PRESETS[presetIndex].ki;
    state->kd = PID_PRESETS[presetIndex].kd;
}

void vTaskLab5PidInput(void *pvParameters) {
    (void)pvParameters;

    s_keypad.init();

    TickType_t lastWake = xTaskGetTickCount();
    const TickType_t period = pdMS_TO_TICKS(TASK_INPUT_PERIOD_MS);

    for (;;) {
        char key = s_keypad.getKey();

        if (key != 0) {
            lab5PidStateLock();
            Lab5PidState *state = lab5PidStateGet();

            switch (key) {
                case 'A':
                    if (state->setpointSource == SETPOINT_SOURCE_POT) {
                        enterManualMode(state);
                        printf("[INPUT] Setpoint source: MANUAL\r\n");
                    } else {
                        state->setpointSource = SETPOINT_SOURCE_POT;
                        state->activeSetpointC = state->potSetpointC;
                        printf("[INPUT] Setpoint source: POT\r\n");
                    }
                    state->editingSetpoint = false;
                    state->inputBufferLen = 0;
                    state->inputBuffer[0] = '\0';
                    break;

                case 'B':
                    enterManualMode(state);
                    state->manualSetpointC = clampFloat(
                        state->manualSetpointC - SETPOINT_STEP_C,
                        SETPOINT_MIN_C,
                        SETPOINT_MAX_C
                    );
                    state->activeSetpointC = state->manualSetpointC;
                    printf("[INPUT] Manual setpoint decreased\r\n");
                    break;

                case 'C':
                    enterManualMode(state);
                    state->manualSetpointC = clampFloat(
                        state->manualSetpointC + SETPOINT_STEP_C,
                        SETPOINT_MIN_C,
                        SETPOINT_MAX_C
                    );
                    state->activeSetpointC = state->manualSetpointC;
                    printf("[INPUT] Manual setpoint increased\r\n");
                    break;

                case 'D': {
                    uint8_t nextPreset = state->pidPresetIndex + 1;
                    if (nextPreset >= PID_PRESET_COUNT) {
                        nextPreset = 0;
                    }
                    applyPidPreset(state, nextPreset);
                    printf("[INPUT] PID preset: %s\r\n",
                           PID_PRESETS[nextPreset].name);
                    break;
                }

                case '*':
                    state->editingSetpoint = false;
                    state->inputBufferLen = 0;
                    state->inputBuffer[0] = '\0';
                    printf("[INPUT] Numeric setpoint entry cancelled\r\n");
                    break;

                case '#':
                    if (state->editingSetpoint && state->inputBufferLen > 0) {
                        state->inputBuffer[state->inputBufferLen] = '\0';
                        int requested = atoi(state->inputBuffer);
                        state->manualSetpointC = clampFloat(
                            (float)requested,
                            SETPOINT_MIN_C,
                            SETPOINT_MAX_C
                        );
                        state->setpointSource = SETPOINT_SOURCE_MANUAL;
                        state->activeSetpointC = state->manualSetpointC;
                        state->editingSetpoint = false;
                        state->inputBufferLen = 0;
                        state->inputBuffer[0] = '\0';
                        printf("[INPUT] Manual setpoint confirmed\r\n");
                    }
                    break;

                default:
                    if (key >= '0' && key <= '9') {
                        if (!state->editingSetpoint) {
                            state->editingSetpoint = true;
                            state->inputBufferLen = 0;
                            state->inputBuffer[0] = '\0';
                        }

                        if (state->inputBufferLen < SETPOINT_INPUT_MAX_DIGITS) {
                            state->inputBuffer[state->inputBufferLen++] = key;
                            state->inputBuffer[state->inputBufferLen] = '\0';
                            printf("[INPUT] Entering setpoint: %s\r\n",
                                   state->inputBuffer);
                        }
                    }
                    break;
            }

            lab5PidStateUnlock();
        }

        vTaskDelayUntil(&lastWake, period);
    }
}
