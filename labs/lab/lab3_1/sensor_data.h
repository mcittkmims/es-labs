/**
 * @file sensor_data.h
 * @brief Lab 3.1 — Shared Data Structures and Synchronization Primitives
 *
 * Declares the inter-task shared data structures, FreeRTOS synchronization
 * handles, hardware pin mapping, and application constants used by all
 * FreeRTOS tasks in the dual-sensor temperature monitoring system.
 *
 * ──────────────────────────────────────────────────────────────────────────
 * Synchronization strategy
 * ──────────────────────────────────────────────────────────────────────────
 *
 *   Mutex (xSensorMutex): protects concurrent read/write access to
 *   g_sensorData and g_alertData across all three tasks.
 *
 *   Binary Semaphore (xNewReadingSemaphore): signals from the acquisition
 *   task to the conditioning task that a new sensor reading is available.
 *
 * ──────────────────────────────────────────────────────────────────────────
 * Data flow
 * ──────────────────────────────────────────────────────────────────────────
 *
 *   Task 1 (Acquisition) ──[mutex + semaphore]──> Task 2 (Conditioning)
 *   Task 2 (Conditioning) ──[mutex]──> Task 3 (Display & Report)
 */

#ifndef SENSOR_DATA_H
#define SENSOR_DATA_H

#include <Arduino.h>
#include <Arduino_FreeRTOS.h>
#include <semphr.h>
#include "ThresholdAlert.h"

// ══════════════════════════════════════════════════════════════════════════
// Hardware Pin Mapping (Arduino Mega 2560)
// ══════════════════════════════════════════════════════════════════════════

/** NTC thermistor analog input (voltage divider midpoint). */
static const uint8_t PIN_ANALOG_SENSOR = A0;

/** DS18B20 OneWire data pin. */
static const uint8_t PIN_DIGITAL_SENSOR = 2;

/** Green LED — system normal indicator. */
static const uint8_t PIN_LED_GREEN = 8;

/** Red LED — analog sensor alert indicator. */
static const uint8_t PIN_LED_RED = 9;

/** Yellow LED — digital sensor alert indicator. */
static const uint8_t PIN_LED_YELLOW = 10;

/** I2C LCD address (16x2 display). */
static const uint8_t LCD_I2C_ADDRESS = 0x27;
static const uint8_t LCD_COLS = 16;
static const uint8_t LCD_ROWS = 2;

// ══════════════════════════════════════════════════════════════════════════
// NTC Thermistor Parameters
// ══════════════════════════════════════════════════════════════════════════

/** Series resistor in the voltage divider (ohms). */
static const uint32_t NTC_SERIES_RESISTANCE = 10000;

/** NTC nominal resistance at 25°C (ohms). */
static const uint32_t NTC_NOMINAL_RESISTANCE = 10000;

/** NTC Beta coefficient. */
static const uint16_t NTC_BETA_COEFFICIENT = 3950;

/** NTC nominal temperature (°C). */
static const float NTC_NOMINAL_TEMP_C = 25.0f;

// ══════════════════════════════════════════════════════════════════════════
// DS18B20 Parameters
// ══════════════════════════════════════════════════════════════════════════

/** DS18B20 measurement resolution (9–12 bits). */
static const uint8_t DS18B20_RESOLUTION = 10;

// ══════════════════════════════════════════════════════════════════════════
// Threshold Alert Parameters
// ══════════════════════════════════════════════════════════════════════════

/** Analog sensor high threshold (°C) — alert triggers above this. */
static const float ANALOG_THRESHOLD_HIGH = 30.0f;

/** Analog sensor low threshold (°C) — alert clears below this. */
static const float ANALOG_THRESHOLD_LOW = 28.0f;

/** Digital sensor high threshold (°C) — alert triggers above this. */
static const float DIGITAL_THRESHOLD_HIGH = 30.0f;

/** Digital sensor low threshold (°C) — alert clears below this. */
static const float DIGITAL_THRESHOLD_LOW = 28.0f;

/** Number of consecutive readings to confirm a state transition. */
static const uint8_t ALERT_DEBOUNCE_COUNT = 5;

// ══════════════════════════════════════════════════════════════════════════
// FreeRTOS Task Configuration
// ══════════════════════════════════════════════════════════════════════════

/** Task 1 — Sensor acquisition: 50 ms period, highest priority. */
static const uint32_t TASK_ACQUISITION_PERIOD_MS = 50;
static const UBaseType_t TASK_ACQUISITION_PRIORITY = 3;
static const configSTACK_DEPTH_TYPE TASK_ACQUISITION_STACK = 256;

/** Task 2 — Signal conditioning & alerting: event-driven, medium priority. */
static const uint32_t TASK_CONDITIONING_PERIOD_MS = 100;
static const UBaseType_t TASK_CONDITIONING_PRIORITY = 2;
static const configSTACK_DEPTH_TYPE TASK_CONDITIONING_STACK = 256;

/** Task 3 — Display & reporting: 500 ms period, lowest priority. */
static const uint32_t TASK_DISPLAY_PERIOD_MS = 500;
static const UBaseType_t TASK_DISPLAY_PRIORITY = 1;
static const configSTACK_DEPTH_TYPE TASK_DISPLAY_STACK = 512;

// ══════════════════════════════════════════════════════════════════════════
// Shared Data Structures
// ══════════════════════════════════════════════════════════════════════════

/**
 * @brief Sensor readings shared between Task 1 and Task 2.
 *
 * Written by Task 1 (Acquisition) under mutex protection.
 * Read by Task 2 (Conditioning) after semaphore notification.
 */
typedef struct {
    // ── Analog sensor (NTC thermistor) ─────────────────────────────────
    uint16_t analogRaw;          /**< Raw ADC value (0–1023).              */
    float    analogResistance;   /**< Calculated NTC resistance (ohms).    */
    float    analogTempC;        /**< Converted temperature (°C).          */
    bool     analogValid;        /**< True if reading is valid.            */

    // ── Digital sensor (DS18B20) ───────────────────────────────────────
    float    digitalTempC;       /**< DS18B20 temperature (°C).            */
    bool     digitalValid;       /**< True if reading is valid.            */

    // ── Metadata ───────────────────────────────────────────────────────
    uint32_t readingCount;       /**< Total number of acquisition cycles.  */
    TickType_t timestamp;        /**< Tick count at last acquisition.      */
} SensorReadings_t;

/**
 * @brief Alert status shared between Task 2 and Task 3.
 *
 * Written by Task 2 (Conditioning) under mutex protection.
 * Read by Task 3 (Display & Report) under mutex protection.
 */
typedef struct {
    // ── Analog sensor alert ────────────────────────────────────────────
    AlertState analogAlertState;     /**< Current FSM state.               */
    uint8_t    analogDebounceCount;  /**< Current debounce counter.        */
    float      analogFilteredTemp;   /**< Last filtered temperature (°C).  */

    // ── Digital sensor alert ───────────────────────────────────────────
    AlertState digitalAlertState;    /**< Current FSM state.               */
    uint8_t    digitalDebounceCount; /**< Current debounce counter.        */
    float      digitalFilteredTemp;  /**< Last filtered temperature (°C).  */

    // ── System status ──────────────────────────────────────────────────
    uint32_t analogAlertCount;       /**< Total analog alert activations.  */
    uint32_t digitalAlertCount;      /**< Total digital alert activations. */
    uint32_t conditioningCycles;     /**< Total conditioning cycles.       */
} AlertStatus_t;

// ══════════════════════════════════════════════════════════════════════════
// External Declarations
// ══════════════════════════════════════════════════════════════════════════

/** Sensor readings — written by Task 1, consumed by Task 2. */
extern SensorReadings_t g_sensorData;

/** Alert status — written by Task 2, read by Task 3. */
extern AlertStatus_t g_alertData;

/**
 * @brief Binary semaphore for new-reading notification (Task 1 → Task 2).
 *
 * Given by Task 1 after each acquisition cycle completes.
 * Taken by Task 2 to wake up and process the new readings.
 */
extern SemaphoreHandle_t xNewReadingSemaphore;

/**
 * @brief Mutex for protecting shared data (g_sensorData, g_alertData).
 *
 * Acquired by any task before reading or writing the shared data
 * structures. Provides priority inheritance to prevent inversion.
 */
extern SemaphoreHandle_t xSensorMutex;

/**
 * @brief Create and initialize all synchronization primitives.
 *
 * Must be called once from setup() before creating any FreeRTOS tasks.
 */
void sensorDataInit();

#endif // SENSOR_DATA_H
