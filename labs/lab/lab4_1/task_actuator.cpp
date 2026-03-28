/**
 * @file task_actuator.cpp
 * @brief Actuator Control Task Implementation — Lab 4.1
 *
 * Runs the signal conditioning and hardware update loop at
 * TASK_ACTUATOR_PERIOD_MS (75 ms) using vTaskDelayUntil for accurate timing.
 *
 * Each iteration:
 *   1. Acquires the shared mutex and reads g_actuator.pendingCmd.
 *   2. Releases the mutex.
 *   3. Calls s_actuator.setRawCommand() and s_actuator.update(), which
 *      runs the BinaryConditioner pipeline (saturation → debounce →
 *      persistent validation) and drives the LED GPIO accordingly.
 *   4. Re-acquires the mutex and writes back the conditioned state,
 *      debouncing status, confirmation count, and toggle counter.
 */

#include "task_actuator.h"
#include "actuator_data.h"
#include <Arduino.h>

// ── Module-level actuator controller (single instance for this task) ──────────

static ActuatorControl s_actuator(PIN_ACTUATOR_LED,
                                   DEBOUNCE_PERIOD_MS,
                                   PERSIST_COUNT);

// ── Task implementation ───────────────────────────────────────────────────────

void vTaskActuator(void *params) {
    (void)params;

    s_actuator.init();

    TickType_t lastWake = xTaskGetTickCount();

    while (true) {
        // ── Step 1: Read latest raw command under mutex ───────────────────
        ActuatorControl::State rawCmd = ActuatorControl::State::OFF;
        if (xSemaphoreTake(g_actuator.mutex, pdMS_TO_TICKS(5)) == pdTRUE) {
            rawCmd = g_actuator.pendingCmd;
            xSemaphoreGive(g_actuator.mutex);
        }

        // ── Step 2: Apply command through signal conditioning pipeline ────
        s_actuator.setRawCommand(rawCmd);
        s_actuator.update();

        // ── Step 3: Write conditioned result back to shared state ─────────
        ActuatorControl::State newState  = s_actuator.getState();
        bool                   debouncing = s_actuator.isDebouncing();
        uint8_t                confirms   = s_actuator.getConfirmCount();

        if (xSemaphoreTake(g_actuator.mutex, pdMS_TO_TICKS(5)) == pdTRUE) {
            if (newState != g_actuator.actuatorState) {
                g_actuator.toggleCount++;
            }
            g_actuator.actuatorState = newState;
            g_actuator.isDebouncing  = debouncing;
            g_actuator.confirmCount  = confirms;
            xSemaphoreGive(g_actuator.mutex);
        }

        vTaskDelayUntil(&lastWake, pdMS_TO_TICKS(TASK_ACTUATOR_PERIOD_MS));
    }
}
