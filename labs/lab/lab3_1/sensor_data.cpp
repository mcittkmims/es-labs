/**
 * @file sensor_data.cpp
 * @brief Lab 3.1 — Shared State Definitions and Synchronization Init
 *
 * Defines the shared data structures and FreeRTOS synchronization
 * primitives used for inter-task communication in the dual-sensor
 * temperature monitoring system.
 */

#include "sensor_data.h"

// ──────────────────────────────────────────────────────────────────────────
// Shared data — zero-initialized at startup
// ──────────────────────────────────────────────────────────────────────────

SensorReadings_t g_sensorData = {
    0,      // analogRaw
    0.0f,   // analogResistance
    0.0f,   // analogTempC
    false,  // analogValid
    0.0f,   // digitalTempC
    false,  // digitalValid
    0,      // readingCount
    0       // timestamp
};

AlertStatus_t g_alertData = {
    ALERT_NORMAL,  // analogAlertState
    0,             // analogDebounceCount
    0.0f,          // analogFilteredTemp
    ALERT_NORMAL,  // digitalAlertState
    0,             // digitalDebounceCount
    0.0f,          // digitalFilteredTemp
    0,             // analogAlertCount
    0,             // digitalAlertCount
    0              // conditioningCycles
};

// ──────────────────────────────────────────────────────────────────────────
// Synchronization primitives — created at runtime
// ──────────────────────────────────────────────────────────────────────────

SemaphoreHandle_t xNewReadingSemaphore = NULL;
SemaphoreHandle_t xSensorMutex         = NULL;

void sensorDataInit() {
    // Binary semaphore starts "empty" — Task 2 will block until
    // Task 1 gives it after completing a sensor acquisition cycle.
    xNewReadingSemaphore = xSemaphoreCreateBinary();

    // Mutex with priority inheritance — prevents priority inversion
    // when Task 3 (low priority) holds the mutex and Task 1 (high
    // priority) attempts to acquire it.
    xSensorMutex = xSemaphoreCreateMutex();
}
