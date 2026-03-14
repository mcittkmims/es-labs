/**
 * @file task_conditioning.cpp
 * @brief Lab 3.2 — Signal Conditioning & Alert Task Implementation (Task 2)
 *
 * Implements the signal conditioning pipeline and threshold alerting for
 * both analog and digital temperature sensors. Unlike Lab 3.1 which feeds
 * raw sensor values directly to the ThresholdAlert FSM, Lab 3.2 interposes
 * a three-stage conditioning pipeline before alerting.
 *
 * ──────────────────────────────────────────────────────────────────────────
 * Processing sequence (each event)
 * ──────────────────────────────────────────────────────────────────────────
 *
 *   1. Take binary semaphore (block until Task 1 signals)
 *   2. Acquire mutex → read raw temperatures from g_sensorData → release
 *   3. For each valid sensor:
 *      a. Run SignalConditioner.process(rawTemp) → conditioned value
 *         Pipeline: saturate → median filter → EWMA
 *      b. Feed conditioned value into ThresholdAlert.update()
 *   4. Acquire mutex → write conditioned values + alert states → release
 *   5. Update LED indicators based on alert states
 *
 * LED mapping:
 *   GREEN LED  = system normal (both sensors below threshold)
 *   RED LED    = analog sensor alert active
 *   YELLOW LED = digital sensor alert active
 */

#include "task_conditioning.h"
#include "sensor_data.h"
#include "SignalConditioner.h"
#include "ThresholdAlert.h"
#include "Led.h"

// ──────────────────────────────────────────────────────────────────────────
// Local signal conditioner instances (owned by this task)
// ──────────────────────────────────────────────────────────────────────────

static SignalConditioner s_analogConditioner(MEDIAN_WINDOW_SIZE, EWMA_ALPHA,
                                             SATURATION_MIN, SATURATION_MAX);

static SignalConditioner s_digitalConditioner(MEDIAN_WINDOW_SIZE, EWMA_ALPHA,
                                              SATURATION_MIN, SATURATION_MAX);

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

    // Conditioning pipeline intermediate results.
    float analogConditioned  = 0.0f;
    float digitalConditioned = 0.0f;

    for (;;) {
        // ── 1. Wait for new sensor data ─────────────────────────────────
        if (xSemaphoreTake(xNewReadingSemaphore, portMAX_DELAY) != pdTRUE) {
            continue;
        }

        // ── 2. Read raw sensor data under mutex ─────────────────────────
        if (xSemaphoreTake(xSensorMutex, pdMS_TO_TICKS(10)) == pdTRUE) {
            analogTemp   = g_sensorData.analogTempRaw;
            digitalTemp  = g_sensorData.digitalTempRaw;
            analogValid  = g_sensorData.analogValid;
            digitalValid = g_sensorData.digitalValid;
            xSemaphoreGive(xSensorMutex);
        } else {
            continue;  // Could not acquire mutex — skip this cycle.
        }

        // ── 3. Apply signal conditioning pipeline ───────────────────────
        AlertState analogState  = ALERT_NORMAL;
        AlertState digitalState = ALERT_NORMAL;

        if (analogValid && !isnan(analogTemp)) {
            analogConditioned = s_analogConditioner.process(analogTemp);
            analogState = s_analogAlert.update(analogConditioned);
        } else {
            // Sensor invalid: flush stale window and reset alert FSM.
            s_analogConditioner.reset();
            s_analogAlert.init();
            analogConditioned = NAN;
        }

        if (digitalValid && !isnan(digitalTemp)) {
            digitalConditioned = s_digitalConditioner.process(digitalTemp);
            digitalState = s_digitalAlert.update(digitalConditioned);
        } else {
            // Sensor invalid: flush stale window and reset alert FSM.
            s_digitalConditioner.reset();
            s_digitalAlert.init();
            digitalConditioned = NAN;
        }

        // ── 4. Write conditioned values and alert status under mutex ────
        if (xSemaphoreTake(xSensorMutex, pdMS_TO_TICKS(10)) == pdTRUE) {
            // Conditioning intermediates for analog sensor.
            if (analogValid) {
                g_sensorData.analogMedian      = s_analogConditioner.getLastMedian();
                g_sensorData.analogEwma        = s_analogConditioner.getLastEwma();
                g_sensorData.analogConditioned = s_analogConditioner.isValid();
            } else {
                g_sensorData.analogMedian      = NAN;
                g_sensorData.analogEwma        = NAN;
                g_sensorData.analogConditioned = false;
            }

            // Conditioning intermediates for digital sensor.
            if (digitalValid) {
                g_sensorData.digitalMedian      = s_digitalConditioner.getLastMedian();
                g_sensorData.digitalEwma        = s_digitalConditioner.getLastEwma();
                g_sensorData.digitalConditioned = s_digitalConditioner.isValid();
            } else {
                g_sensorData.digitalMedian      = NAN;
                g_sensorData.digitalEwma        = NAN;
                g_sensorData.digitalConditioned = false;
            }

            // Alert status.
            g_alertData.analogAlertState    = analogState;
            g_alertData.analogDebounceCount = s_analogAlert.getDebounceCounter();
            g_alertData.analogCondTemp      = analogConditioned;

            g_alertData.digitalAlertState    = digitalState;
            g_alertData.digitalDebounceCount = s_digitalAlert.getDebounceCounter();
            g_alertData.digitalCondTemp      = digitalConditioned;

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

        // ── 5. Update LED indicators ────────────────────────────────────
        bool anyNonNormal = (analogState  != ALERT_NORMAL) ||
                            (digitalState != ALERT_NORMAL);

        // Green LED: ON only when both sensors are fully NORMAL.
        if (anyNonNormal) {
            s_greenLed.turnOff();
        } else {
            s_greenLed.turnOn();
        }

        // Red LED: ON when analog sensor alert is active.
        if (analogState == ALERT_ACTIVE) {
            s_redLed.turnOn();
        } else if (analogState == ALERT_DEBOUNCE_HIGH ||
                   analogState == ALERT_DEBOUNCE_LOW) {
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
