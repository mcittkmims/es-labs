/**
 * @file shared_state.cpp
 * @brief Lab 4 — Shared State Implementation
 */

#include "shared_state.h"
#include <string.h>

static ActuatorState _state;
static SemaphoreHandle_t _mutex = NULL;

void sharedStateInit() {
    memset(&_state, 0, sizeof(_state));
    _state.relayCommandOn = false;
    _state.relayActualOn = false;
    _state.pwmCommandPercent = 0.0f;
    _state.inputModeAnalog = false;
    _state.inputBufferLen = 0;
    _mutex = xSemaphoreCreateMutex();
}

void sharedStateLock() {
    xSemaphoreTake(_mutex, portMAX_DELAY);
}

void sharedStateUnlock() {
    xSemaphoreGive(_mutex);
}

ActuatorState* sharedStateGet() {
    return &_state;
}
