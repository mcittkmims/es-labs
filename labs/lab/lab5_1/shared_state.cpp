/**
 * @file shared_state.cpp
 * @brief Lab 5.1 shared state implementation.
 */

#include "shared_state.h"
#include "lab5_1_config.h"
#include <string.h>

Lab5ControlState g_lab5State;
SemaphoreHandle_t xLab5StateMutex = NULL;
SemaphoreHandle_t xLab5NewSampleSemaphore = NULL;
SemaphoreHandle_t xLab5ActuatorSemaphore = NULL;

void lab5StateInit() {
    memset(&g_lab5State, 0, sizeof(g_lab5State));

    g_lab5State.measuredTempC = NAN;
    g_lab5State.measuredHumidityPercent = NAN;
    g_lab5State.sensorValid = false;
    g_lab5State.manualSetpointC = SETPOINT_DEFAULT_C;
    g_lab5State.activeSetpointC = SETPOINT_DEFAULT_C;
    g_lab5State.potSetpointC = SETPOINT_DEFAULT_C;
    g_lab5State.setpointSource = SETPOINT_SOURCE_POT;
    g_lab5State.hysteresisBandC = HYSTERESIS_DEFAULT_C;
    g_lab5State.lowerThresholdC = SETPOINT_DEFAULT_C - HYSTERESIS_DEFAULT_C * 0.5f;
    g_lab5State.upperThresholdC = SETPOINT_DEFAULT_C + HYSTERESIS_DEFAULT_C * 0.5f;
    g_lab5State.controlCommandOn = false;
    g_lab5State.actuatorOn = false;
    g_lab5State.controllerState = HYSTERESIS_OUTPUT_OFF;
    g_lab5State.editingSetpoint = false;
    g_lab5State.inputBuffer[0] = '\0';
    g_lab5State.inputBufferLen = 0;

    xLab5StateMutex = xSemaphoreCreateMutex();
    xLab5NewSampleSemaphore = xSemaphoreCreateBinary();
    xLab5ActuatorSemaphore = xSemaphoreCreateBinary();
}

void lab5StateLock() {
    xSemaphoreTake(xLab5StateMutex, portMAX_DELAY);
}

void lab5StateUnlock() {
    xSemaphoreGive(xLab5StateMutex);
}

Lab5ControlState* lab5StateGet() {
    return &g_lab5State;
}
