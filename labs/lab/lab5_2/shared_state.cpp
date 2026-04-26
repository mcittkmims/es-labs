/**
 * @file shared_state.cpp
 * @brief Lab 5.2 shared state implementation.
 */

#include "shared_state.h"
#include <math.h>
#include <string.h>

Lab5PidState g_lab5PidState;
SemaphoreHandle_t xLab5PidStateMutex = NULL;
SemaphoreHandle_t xLab5PidNewSampleSemaphore = NULL;
SemaphoreHandle_t xLab5PidActuatorSemaphore = NULL;

void lab5PidStateInit() {
    memset(&g_lab5PidState, 0, sizeof(g_lab5PidState));

    g_lab5PidState.measuredTempC = NAN;
    g_lab5PidState.measuredHumidityPercent = NAN;
    g_lab5PidState.sensorValid = false;
    g_lab5PidState.potRaw = 0;
    g_lab5PidState.potSetpointC = SETPOINT_DEFAULT_C;
    g_lab5PidState.manualSetpointC = SETPOINT_DEFAULT_C;
    g_lab5PidState.activeSetpointC = SETPOINT_DEFAULT_C;
    g_lab5PidState.setpointSource = SETPOINT_SOURCE_POT;

    g_lab5PidState.pidPresetIndex = 1;
    g_lab5PidState.kp = PID_PRESETS[g_lab5PidState.pidPresetIndex].kp;
    g_lab5PidState.ki = PID_PRESETS[g_lab5PidState.pidPresetIndex].ki;
    g_lab5PidState.kd = PID_PRESETS[g_lab5PidState.pidPresetIndex].kd;

    g_lab5PidState.errorC = 0.0f;
    g_lab5PidState.pidIntegral = 0.0f;
    g_lab5PidState.pidDerivative = 0.0f;
    g_lab5PidState.controlOutputPercent = 0.0f;
    g_lab5PidState.appliedDutyPercent = 0.0f;
    g_lab5PidState.fanRunning = false;
    g_lab5PidState.editingSetpoint = false;
    g_lab5PidState.inputBuffer[0] = '\0';
    g_lab5PidState.inputBufferLen = 0;

    xLab5PidStateMutex = xSemaphoreCreateMutex();
    xLab5PidNewSampleSemaphore = xSemaphoreCreateBinary();
    xLab5PidActuatorSemaphore = xSemaphoreCreateBinary();
}

void lab5PidStateLock() {
    xSemaphoreTake(xLab5PidStateMutex, portMAX_DELAY);
}

void lab5PidStateUnlock() {
    xSemaphoreGive(xLab5PidStateMutex);
}

Lab5PidState* lab5PidStateGet() {
    return &g_lab5PidState;
}
