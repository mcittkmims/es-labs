/**
 * @file shared_state.h
 * @brief Lab 4 — Shared State and Synchronization
 *
 * Defines the shared data structure for inter-task communication
 * and provides mutex-protected access functions. All tasks exchange
 * actuator commands and status through this module.
 */

#ifndef SHARED_STATE_H
#define SHARED_STATE_H

#include <Arduino.h>
#include <Arduino_FreeRTOS.h>
#include <semphr.h>

/**
 * @struct ActuatorState
 * @brief Shared data exchanged between input, control, and display tasks.
 */
struct ActuatorState {
    // Binary actuator (relay)
    bool relayCommandOn;       // Desired relay state from user
    bool relayActualOn;        // Actual relay state after debounce
    uint8_t relayDebounceCount; // Current debounce counter

    // Analog actuator (PWM)
    float pwmCommandPercent;   // Raw target from user input (0-100)
    float pwmConditioned;      // After conditioning pipeline
    float pwmRamped;           // After ramping (actual output)
    uint8_t pwmRawValue;       // PWM register value (0-255)

    // Alert state
    bool overloadAlert;        // True if analog actuator overloaded

    // Input mode tracking
    bool inputModeAnalog;      // True = editing analog value, false = binary
    char inputBuffer[4];       // Numeric input buffer for PWM %
    uint8_t inputBufferLen;    // Current length of input buffer
};

/** @brief Initialize shared state and create the mutex. */
void sharedStateInit();

/**
 * @brief Lock the shared state mutex (blocking).
 * Must be followed by sharedStateUnlock().
 */
void sharedStateLock();

/** @brief Unlock the shared state mutex. */
void sharedStateUnlock();

/** @brief Get a pointer to the shared state (only access while locked). */
ActuatorState* sharedStateGet();

#endif // SHARED_STATE_H
