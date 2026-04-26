/**
 * @file shared_state.h
 * @brief Lab 5.2 shared PID control state.
 */

#ifndef LAB5_2_SHARED_STATE_H
#define LAB5_2_SHARED_STATE_H

#include <Arduino.h>
#include <Arduino_FreeRTOS.h>
#include <semphr.h>
#include "lab5_2_config.h"

enum SetpointSource {
    SETPOINT_SOURCE_POT = 0,
    SETPOINT_SOURCE_MANUAL = 1
};

struct Lab5PidState {
    float measuredTempC;
    float measuredHumidityPercent;
    bool sensorValid;
    uint16_t potRaw;
    float potSetpointC;
    float manualSetpointC;
    float activeSetpointC;
    SetpointSource setpointSource;

    float kp;
    float ki;
    float kd;
    uint8_t pidPresetIndex;

    float errorC;
    float pidIntegral;
    float pidDerivative;
    float controlOutputPercent;
    float appliedDutyPercent;
    bool fanRunning;

    bool editingSetpoint;
    char inputBuffer[SETPOINT_INPUT_MAX_DIGITS + 1];
    uint8_t inputBufferLen;

    uint32_t sampleCount;
    uint32_t controlCycles;
    uint32_t actuatorUpdates;
    TickType_t lastSampleTick;
};

extern Lab5PidState g_lab5PidState;
extern SemaphoreHandle_t xLab5PidStateMutex;
extern SemaphoreHandle_t xLab5PidNewSampleSemaphore;
extern SemaphoreHandle_t xLab5PidActuatorSemaphore;

void lab5PidStateInit();
void lab5PidStateLock();
void lab5PidStateUnlock();
Lab5PidState* lab5PidStateGet();

#endif // LAB5_2_SHARED_STATE_H
