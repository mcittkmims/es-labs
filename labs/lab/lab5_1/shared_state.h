/**
 * @file shared_state.h
 * @brief Lab 5.1 shared state and synchronization primitives.
 */

#ifndef LAB5_1_SHARED_STATE_H
#define LAB5_1_SHARED_STATE_H

#include <Arduino.h>
#include <Arduino_FreeRTOS.h>
#include <semphr.h>
#include "lab5_1_config.h"
#include "OnOffHysteresisController.h"

enum SetpointSource {
    SETPOINT_SOURCE_POT = 0,
    SETPOINT_SOURCE_MANUAL = 1
};

struct Lab5ControlState {
    float measuredTempC;
    float measuredHumidityPercent;
    bool sensorValid;
    uint16_t potRaw;
    float potSetpointC;
    float manualSetpointC;
    float activeSetpointC;
    SetpointSource setpointSource;

    float hysteresisBandC;
    float lowerThresholdC;
    float upperThresholdC;
    bool controlCommandOn;
    bool actuatorOn;
    HysteresisOutputState controllerState;

    bool editingSetpoint;
    char inputBuffer[SETPOINT_INPUT_MAX_DIGITS + 1];
    uint8_t inputBufferLen;

    uint32_t sampleCount;
    uint32_t controlCycles;
    uint32_t actuatorSwitches;
    TickType_t lastSampleTick;
};

extern Lab5ControlState g_lab5State;
extern SemaphoreHandle_t xLab5StateMutex;
extern SemaphoreHandle_t xLab5NewSampleSemaphore;
extern SemaphoreHandle_t xLab5ActuatorSemaphore;

void lab5StateInit();
void lab5StateLock();
void lab5StateUnlock();
Lab5ControlState* lab5StateGet();

#endif // LAB5_1_SHARED_STATE_H
