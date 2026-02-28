/**
 * @file task_measure.cpp
 * @brief Lab 2.2 — Task 1 Implementation: Button FSM and LED Signaling
 *
 * Implements the highest-priority FreeRTOS task that polls the button every
 * 10 ms, debounces press and release edges using a four-state FSM, measures
 * the press duration in milliseconds, and signals the result to Task 2 via
 * a binary semaphore. Visual feedback is provided by lighting a green LED
 * (short press) or red LED (long press) for a configurable timeout.
 *
 * The debounce FSM is identical to the bare-metal Lab 2.1 implementation,
 * demonstrating that the core application logic ports to FreeRTOS with
 * minimal changes — only the scheduling mechanism and inter-task
 * communication differ.
 *
 * FSM states:
 *   BTN_IDLE         → Button not pressed, waiting for LOW edge.
 *   BTN_DEBOUNCE_DOWN → Button read LOW, confirming for DEBOUNCE_MS.
 *   BTN_PRESSED      → Press confirmed, measuring duration until release.
 *   BTN_DEBOUNCE_UP  → Button read HIGH, confirming release for DEBOUNCE_MS.
 */

#include "task_measure.h"
#include "shared_state.h"

#include <Arduino.h>
#include <Arduino_FreeRTOS.h>
#include <semphr.h>

// ──────────────────────────────────────────────────────────────────────────
// Button FSM states
// ──────────────────────────────────────────────────────────────────────────

typedef enum {
    BTN_IDLE,           /**< No button activity detected.                       */
    BTN_DEBOUNCE_DOWN,  /**< Button went LOW; waiting for debounce confirmation. */
    BTN_PRESSED,        /**< Button confirmed pressed; measuring duration.       */
    BTN_DEBOUNCE_UP     /**< Button went HIGH; waiting for release confirmation. */
} ButtonState_e;

// ──────────────────────────────────────────────────────────────────────────
// Task 1 implementation
// ──────────────────────────────────────────────────────────────────────────

void vTaskMeasure(void *pvParameters) {
    (void)pvParameters;  // Unused

    // ── Task-local state (private to this task — no sharing needed) ────
    ButtonState_e btnState      = BTN_IDLE;
    uint32_t      debounceStart = 0;
    uint32_t      pressStart    = 0;
    uint32_t      pressEnd      = 0;
    uint32_t      greenLedOffAt = 0;  // millis() when green LED should turn off
    uint32_t      redLedOffAt   = 0;  // millis() when red LED should turn off

    for (;;) {
        uint32_t now    = millis();
        bool     btnLow = (digitalRead(PIN_BUTTON) == LOW);

        // ── FSM update ─────────────────────────────────────────────────
        switch (btnState) {

            case BTN_IDLE:
                if (btnLow) {
                    debounceStart = now;
                    btnState      = BTN_DEBOUNCE_DOWN;
                }
                break;

            case BTN_DEBOUNCE_DOWN:
                if (!btnLow) {
                    // Spurious glitch — button released before debounce expired.
                    btnState = BTN_IDLE;
                } else if ((now - debounceStart) >= DEBOUNCE_MS) {
                    // Confirmed press — record start time.
                    pressStart = now;
                    btnState   = BTN_PRESSED;
                }
                break;

            case BTN_PRESSED:
                if (!btnLow) {
                    // Button released — begin release debounce.
                    pressEnd      = now;
                    debounceStart = now;
                    btnState      = BTN_DEBOUNCE_UP;
                }
                break;

            case BTN_DEBOUNCE_UP:
                if (btnLow) {
                    // Bounce — button went back down, still considered pressed.
                    btnState = BTN_PRESSED;
                } else if ((now - debounceStart) >= DEBOUNCE_MS) {
                    // Confirmed release — compute duration and signal result.
                    uint32_t duration = pressEnd - pressStart;
                    bool     isShort  = (duration < SHORT_PRESS_THRESHOLD_MS);

                    // Write press info under mutex protection, then signal Task 2.
                    if (xSemaphoreTake(xSharedDataMutex, portMAX_DELAY) == pdTRUE) {
                        g_pressInfo.duration = duration;
                        g_pressInfo.isShort  = isShort;
                        xSemaphoreGive(xSharedDataMutex);
                    }

                    // Signal Task 2 that a new press event is available.
                    xSemaphoreGive(xNewPressSemaphore);

                    // Light the appropriate indicator LED.
                    if (isShort) {
                        digitalWrite(PIN_LED_GREEN, HIGH);
                        greenLedOffAt = now + LED_INDICATOR_DURATION_MS;
                    } else {
                        digitalWrite(PIN_LED_RED, HIGH);
                        redLedOffAt = now + LED_INDICATOR_DURATION_MS;
                    }

                    btnState = BTN_IDLE;
                }
                break;
        }

        // ── Auto-off for indicator LEDs ────────────────────────────────
        if (greenLedOffAt != 0 && now >= greenLedOffAt) {
            digitalWrite(PIN_LED_GREEN, LOW);
            greenLedOffAt = 0;
        }
        if (redLedOffAt != 0 && now >= redLedOffAt) {
            digitalWrite(PIN_LED_RED, LOW);
            redLedOffAt = 0;
        }

        // ── Yield CPU until next 10 ms tick ────────────────────────────
        vTaskDelay(pdMS_TO_TICKS(TASK_MEASURE_PERIOD_MS));
    }
}
