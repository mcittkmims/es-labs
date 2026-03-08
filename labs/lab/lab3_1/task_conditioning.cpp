/**
 * @file task_conditioning.cpp
 * @brief Lab 3.1 — Signal Conditioning & Alert Task Implementation (Task 2)
 *
 * Implements the threshold-based signal conditioning and alert management
 * for both analog and digital temperature sensors. Uses the ThresholdAlert
 * module for hysteresis detection and debounce filtering.
 *
 * ──────────────────────────────────────────────────────────────────────────
 * Processing sequence (each event)
 * ──────────────────────────────────────────────────────────────────────────
 *
 *   1. Take binary semaphore (block until Task 1 signals)
 *   2. Acquire mutex → read g_sensorData → release mutex
 *   3. Feed analog temperature into ThresholdAlert FSM
 *   4. Feed digital temperature into ThresholdAlert FSM
 *   5. Acquire mutex → write g_alertData → release mutex
 *   6. Update LED indicators based on alert states
 *
 * LED mapping:
 *   GREEN LED  = system normal (both sensors below threshold)
 *   RED LED    = analog sensor alert active
 *   YELLOW LED = digital sensor alert active
 */

#include "task_conditioning.h"
#include "sensor_data.h"
#include "ThresholdAlert.h"
#include "Led.h"

// ──────────────────────────────────────────────────────────────────────────
// Local alert instances (owned by this task)
// ──────────────────────────────────────────────────────────────────────────

static ThresholdAlert s_analogAlert(ANALOG_THRESHOLD_HIGH,
                                    ANALOG_THRESHOLD_LOW,
                                    ALERT_DEBOUNCE_COUNT);

static ThresholdAlert s_digitalAlert(DIGITAL_THRESHOLD_HIGH,
                                     DIGITAL_THRESHOLD_LOW,
                                     ALERT_DEBOUNCE_COUNT);

// ──────────────────────────────────────────────────────────────────────────
// Local LED instances
// ──────────────────────────────────────────────────────────────────────────

static Led s_greenLed(PIN_LED_GREEN);
static Led s_redLed(PIN_LED_RED);
static Led s_yellowLed(PIN_LED_YELLOW);

// ──────────────────────────────────────────────────────────────────────────
// Task function
// ──────────────────────────────────────────────────────────────────────────

void vTaskConditioning(void *pvParameters) {
    (void)pvParameters;

    // Initialize alert modules and LEDs.
    s_analogAlert.init();
    s_digitalAlert.init();
    s_greenLed.init();
    s_redLed.init();
    s_yellowLed.init();

    // Start with green LED on (system normal).
    s_greenLed.turnOn();
    s_redLed.turnOff();
    s_yellowLed.turnOff();

    // Local copies of sensor readings (avoid holding mutex during processing).
    float analogTemp;
    float digitalTemp;
    bool  analogValid;
    bool  digitalValid;

    // Track previous alert states for transition counting.
    AlertState prevAnalogState  = ALERT_NORMAL;
    AlertState prevDigitalState = ALERT_NORMAL;

    for (;;) {
        // ── 1. Wait for new sensor data ───────────────────────────────
        if (xSemaphoreTake(xNewReadingSemaphore, portMAX_DELAY) != pdTRUE) {
            continue;
        }

        // ── 2. Read sensor data under mutex ───────────────────────────
        if (xSemaphoreTake(xSensorMutex, pdMS_TO_TICKS(10)) == pdTRUE) {
            analogTemp   = g_sensorData.analogTempC;
            digitalTemp  = g_sensorData.digitalTempC;
            analogValid  = g_sensorData.analogValid;
            digitalValid = g_sensorData.digitalValid;
            xSemaphoreGive(xSensorMutex);
        } else {
            continue;  // Could not acquire mutex — skip this cycle.
        }

        // ── 3. Apply threshold detection with hysteresis ──────────────
        AlertState analogState  = ALERT_NORMAL;
        AlertState digitalState = ALERT_NORMAL;

        if (analogValid && !isnan(analogTemp)) {
            analogState = s_analogAlert.update(analogTemp);
        }

        if (digitalValid && !isnan(digitalTemp)) {
            digitalState = s_digitalAlert.update(digitalTemp);
        }

        // ── 4. Write alert status under mutex ─────────────────────────
        if (xSemaphoreTake(xSensorMutex, pdMS_TO_TICKS(10)) == pdTRUE) {
            g_alertData.analogAlertState    = analogState;
            g_alertData.analogDebounceCount = s_analogAlert.getDebounceCounter();
            g_alertData.analogFilteredTemp  = analogTemp;

            g_alertData.digitalAlertState    = digitalState;
            g_alertData.digitalDebounceCount = s_digitalAlert.getDebounceCounter();
            g_alertData.digitalFilteredTemp  = digitalTemp;

            // Count new alert activations.
            if (analogState == ALERT_ACTIVE && prevAnalogState != ALERT_ACTIVE) {
                g_alertData.analogAlertCount++;
            }
            if (digitalState == ALERT_ACTIVE && prevDigitalState != ALERT_ACTIVE) {
                g_alertData.digitalAlertCount++;
            }

            g_alertData.conditioningCycles++;

            xSemaphoreGive(xSensorMutex);
        }

        // Remember current states for next-cycle transition detection.
        prevAnalogState  = analogState;
        prevDigitalState = digitalState;

        // ── 5. Update LED indicators ──────────────────────────────────
        bool anyAlert = (analogState == ALERT_ACTIVE) ||
                        (digitalState == ALERT_ACTIVE);

        // Green LED: ON when no alerts, OFF when any alert active.
        if (anyAlert) {
            s_greenLed.turnOff();
        } else {
            s_greenLed.turnOn();
        }

        // Red LED: ON when analog sensor alert is active.
        if (analogState == ALERT_ACTIVE) {
            s_redLed.turnOn();
        } else if (analogState == ALERT_DEBOUNCE_HIGH ||
                   analogState == ALERT_DEBOUNCE_LOW) {
            // Blink during debounce to indicate transitioning state.
            s_redLed.toggle();
        } else {
            s_redLed.turnOff();
        }

        // Yellow LED: ON when digital sensor alert is active.
        if (digitalState == ALERT_ACTIVE) {
            s_yellowLed.turnOn();
        } else if (digitalState == ALERT_DEBOUNCE_HIGH ||
                   digitalState == ALERT_DEBOUNCE_LOW) {
            s_yellowLed.toggle();
        } else {
            s_yellowLed.turnOff();
        }
    }
}
