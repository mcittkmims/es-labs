/**
 * @file actuator_data.cpp
 * @brief Shared Actuator State — Initialization
 */

#include "actuator_data.h"

// ── Global shared state instance ─────────────────────────────────────────────

ActuatorShared g_actuator;

// ── Initialization ────────────────────────────────────────────────────────────

void actuatorDataInit() {
    g_actuator.pendingCmd    = ActuatorControl::State::OFF;
    g_actuator.actuatorState = ActuatorControl::State::OFF;
    g_actuator.isDebouncing  = false;
    g_actuator.confirmCount  = 0;
    g_actuator.toggleCount   = 0;
    g_actuator.mutex         = xSemaphoreCreateMutex();
}
