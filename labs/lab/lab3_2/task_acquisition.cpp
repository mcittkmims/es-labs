/**
 * @file task_acquisition.cpp
 * @brief Lab 3.2 — Sensor Acquisition Task Implementation (Task 1)
 *
 * Implements the periodic acquisition of temperature data from two
 * sensors: an NTC thermistor (analog, via ADC) and a DS18B20 (digital,
 * via OneWire). Runs at 50 ms intervals using vTaskDelayUntil() for
 * drift-free timing.
 *
 * ──────────────────────────────────────────────────────────────────────────
 * Acquisition sequence (each 50 ms cycle)
 * ──────────────────────────────────────────────────────────────────────────
 *
 *   1. Read NTC thermistor via analogRead() → convert to °C (Beta eq.)
 *   2. Read DS18B20 via DallasTemperature → get °C directly
 *   3. Acquire mutex → write raw values to g_sensorData → release mutex
 *   4. Give binary semaphore → wake up Task 2 (conditioning)
 *
 * This task writes only raw sensor values. Signal conditioning
 * (saturation, median filter, EWMA) is handled by Task 2.
 */

#include "task_acquisition.h"
#include "sensor_data.h"

#include "AnalogTempSensor.h"
#include "DigitalTempSensor.h"

// ──────────────────────────────────────────────────────────────────────────
// Local sensor instances (owned by this task — no sharing needed)
// ──────────────────────────────────────────────────────────────────────────

static AnalogTempSensor  s_ntcSensor(PIN_ANALOG_SENSOR,
                                     NTC_SERIES_RESISTANCE,
                                     NTC_NOMINAL_RESISTANCE,
                                     NTC_BETA_COEFFICIENT,
                                     NTC_NOMINAL_TEMP_C);

static DigitalTempSensor s_ds18b20(PIN_DIGITAL_SENSOR, DS18B20_RESOLUTION);

// ──────────────────────────────────────────────────────────────────────────
// Task function
// ──────────────────────────────────────────────────────────────────────────

void vTaskAcquisition(void *pvParameters) {
    (void)pvParameters;

    // Initialize sensor hardware.
    s_ntcSensor.init();
    bool ds18b20Found = s_ds18b20.init();

    // Kick off the first DS18B20 conversion so the next read has data.
    if (ds18b20Found) {
        s_ds18b20.requestConversion();
    }

    // Track whether the first DS18B20 conversion is still pending.
    bool firstConversion = true;

    TickType_t xLastWakeTime = xTaskGetTickCount();
    const TickType_t xPeriod = pdMS_TO_TICKS(TASK_ACQUISITION_PERIOD_MS);

    // Local variables for readings (avoid holding mutex during I/O).
    uint16_t rawAdc;
    float    resistance;
    float    analogTemp;
    bool     analogOk;
    float    digitalTemp;
    bool     digitalOk;

    for (;;) {
        vTaskDelayUntil(&xLastWakeTime, xPeriod);

        // ── 1. Read analog sensor (NTC thermistor) ──────────────────────
        analogTemp  = s_ntcSensor.readTemperatureC();
        rawAdc      = s_ntcSensor.getLastRaw();
        resistance  = s_ntcSensor.getLastResistance();
        analogOk    = s_ntcSensor.isValid();

        // ── 2. Read digital sensor (DS18B20) ────────────────────────────
        if (ds18b20Found) {
            if (firstConversion) {
                // Wait for the initial conversion to complete.
                while (!s_ds18b20.isConversionComplete()) {
                    vTaskDelay(pdMS_TO_TICKS(10));
                }
                firstConversion = false;
            }

            if (s_ds18b20.isConversionComplete()) {
                digitalTemp = s_ds18b20.readLastConversionC();
                digitalOk   = s_ds18b20.isValid();
                // Conversion is done — start the next one.
                s_ds18b20.requestConversion();
            } else {
                // Still converting — keep last known-good value, don't restart.
                digitalTemp = s_ds18b20.getLastTemperatureC();
                digitalOk   = s_ds18b20.isValid();
            }
        } else {
            digitalTemp = NAN;
            digitalOk   = false;
        }

        // ── 3. Write raw values to shared data under mutex ──────────────
        if (xSemaphoreTake(xSensorMutex, pdMS_TO_TICKS(10)) == pdTRUE) {
            g_sensorData.analogRaw        = rawAdc;
            g_sensorData.analogResistance = resistance;
            g_sensorData.analogTempRaw    = analogTemp;
            g_sensorData.analogValid      = analogOk;

            g_sensorData.digitalTempRaw   = digitalTemp;
            g_sensorData.digitalValid     = digitalOk;

            g_sensorData.readingCount++;
            g_sensorData.timestamp = xTaskGetTickCount();

            xSemaphoreGive(xSensorMutex);
        }

        // ── 4. Notify Task 2 that new data is available ─────────────────
        xSemaphoreGive(xNewReadingSemaphore);
    }
}
