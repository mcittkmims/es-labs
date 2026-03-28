/**
 * @file actuator_data.h
 * @brief Shared Actuator State — Inter-Task Communication for Lab 4.1
 *
 * Defines the mutex-protected shared state structure used by the three
 * FreeRTOS tasks to exchange actuator command and status data:
 *
 *   Task 1 (Input Reader)     — writes: pendingCmd
 *   Task 2 (Actuator Control) — reads:  pendingCmd
 *                             — writes: actuatorState, isDebouncing, confirmCount
 *   Task 3 (Display/Report)   — reads:  all fields
 *
 * Timing and stack constants for all three tasks are also defined here
 * to provide a single configuration point for the system parameters.
 */

#ifndef ACTUATOR_DATA_H
#define ACTUATOR_DATA_H

#include <Arduino_FreeRTOS.h>
#include <semphr.h>
#include "ActuatorControl.h"

// ── Task timing constants ─────────────────────────────────────────────────────
#define TASK_INPUT_PERIOD_MS      10U   ///< Input polling interval (ms)
#define TASK_ACTUATOR_PERIOD_MS   75U   ///< Actuator control recurrence (ms)
#define TASK_DISPLAY_PERIOD_MS   500U   ///< Display/reporting recurrence (ms)
#define STDIO_REPORT_EVERY_N       4U   ///< Print STDIO report every N display ticks

// ── Task stack sizes (words) ──────────────────────────────────────────────────
#define TASK_INPUT_STACK         192U
#define TASK_ACTUATOR_STACK      256U
#define TASK_DISPLAY_STACK       320U

// ── Task priorities ───────────────────────────────────────────────────────────
#define TASK_INPUT_PRIORITY        3
#define TASK_ACTUATOR_PRIORITY     2
#define TASK_DISPLAY_PRIORITY      1

// ── Hardware pin mapping ──────────────────────────────────────────────────────
#define PIN_ACTUATOR_LED           7U   ///< LED actuator output (D7, simulates relay)

// ── Signal conditioning parameters ───────────────────────────────────────────
#define DEBOUNCE_PERIOD_MS       100U   ///< Debounce window before accepting change (ms)
#define PERSIST_COUNT              3U   ///< Consecutive confirmations required

/**
 * @struct ActuatorShared
 * @brief Mutex-protected shared state for inter-task communication.
 *
 * All fields must be accessed under the mutex to prevent race conditions
 * between the input reader, actuator control, and display tasks.
 */
typedef struct {
    ActuatorControl::State pendingCmd;    ///< Latest raw command from input task
    ActuatorControl::State actuatorState; ///< Current conditioned hardware state
    bool                   isDebouncing;  ///< True while validating a transition
    uint8_t                confirmCount;  ///< Confirmations so far for pending change
    uint32_t               toggleCount;  ///< Total number of committed state changes
    SemaphoreHandle_t      mutex;         ///< Mutex protecting all fields above
} ActuatorShared;

/** @brief Global shared actuator state instance (defined in actuator_data.cpp). */
extern ActuatorShared g_actuator;

/**
 * @brief Initialize the shared state structure and create the FreeRTOS mutex.
 *
 * Must be called once from lab4_1Setup() before any tasks are created.
 */
void actuatorDataInit();

#endif // ACTUATOR_DATA_H
