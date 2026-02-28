/**
 * @file shared_state.h
 * @brief Lab 2.2 — Shared State and Synchronization Primitives
 *
 * Declares the inter-task shared data structures, FreeRTOS synchronization
 * handles, hardware pin mapping, and application constants used across all
 * three FreeRTOS tasks in the button press monitoring system.
 *
 * Synchronization strategy:
 *   - Binary semaphore (xNewPressSemaphore): signals Task 2 that a new
 *     press event is available from Task 1.
 *   - Mutex (xSharedDataMutex): protects concurrent read/write access to
 *     g_pressInfo and g_stats from Task 1, Task 2, and Task 3.
 *
 * Data flow:
 *   Task 1 (measure) → [mutex + semaphore] → Task 2 (stats) → [mutex] → Task 3 (report)
 */

#ifndef SHARED_STATE_H
#define SHARED_STATE_H

#include <Arduino.h>
#include <Arduino_FreeRTOS.h>
#include <semphr.h>

// ──────────────────────────────────────────────────────────────────────────
// Hardware Pin Mapping (Arduino Mega 2560)
// ──────────────────────────────────────────────────────────────────────────

static const uint8_t PIN_BUTTON     = 7;   /**< Push button (active-LOW, INPUT_PULLUP) */
static const uint8_t PIN_LED_GREEN  = 8;   /**< Green LED — short press indicator      */
static const uint8_t PIN_LED_RED    = 9;   /**< Red LED   — long press indicator       */
static const uint8_t PIN_LED_YELLOW = 10;  /**< Yellow LED — activity blink sequencer  */

// ──────────────────────────────────────────────────────────────────────────
// Application Constants
// ──────────────────────────────────────────────────────────────────────────

/** A press shorter than this value (ms) is classified as "short". */
static const uint32_t SHORT_PRESS_THRESHOLD_MS  = 500;

/** How long the green/red LED stays on after a completed press (ms). */
static const uint32_t LED_INDICATOR_DURATION_MS  = 1500;

/** Half-period for one yellow LED blink step (ms). Each blink = ON + OFF. */
static const uint32_t BLINK_HALF_PERIOD_MS       = 100;

/** Number of complete on-off blinks for a short press. */
static const uint8_t  BLINK_COUNT_SHORT          = 5;

/** Number of complete on-off blinks for a long press. */
static const uint8_t  BLINK_COUNT_LONG           = 10;

/** Button debounce confirmation period (ms). */
static const uint32_t DEBOUNCE_MS                = 50;

// ──────────────────────────────────────────────────────────────────────────
// FreeRTOS Task Configuration
// ──────────────────────────────────────────────────────────────────────────

/** Task 1 — Button measurement: 10 ms period, highest priority. */
static const uint32_t TASK_MEASURE_PERIOD_MS = 10;
static const UBaseType_t TASK_MEASURE_PRIORITY = 3;
static const configSTACK_DEPTH_TYPE TASK_MEASURE_STACK = 200;

/** Task 2 — Statistics and blink: event-driven (semaphore), medium priority. */
static const UBaseType_t TASK_STATS_PRIORITY = 2;
static const configSTACK_DEPTH_TYPE TASK_STATS_STACK = 200;

/** Task 3 — Periodic report: 10 s period, lowest priority. */
static const uint32_t TASK_REPORT_PERIOD_MS = 10000;
static const UBaseType_t TASK_REPORT_PRIORITY = 1;
static const configSTACK_DEPTH_TYPE TASK_REPORT_STACK = 300;

// ──────────────────────────────────────────────────────────────────────────
// Shared Data Structures
// ──────────────────────────────────────────────────────────────────────────

/**
 * @brief Press event information passed from Task 1 to Task 2.
 *
 * Written by Task 1 under mutex protection before giving the binary
 * semaphore. Read by Task 2 under mutex protection after taking the
 * semaphore.
 */
typedef struct {
    uint32_t duration;   /**< Duration of the completed press in ms. */
    bool     isShort;    /**< True if duration < SHORT_PRESS_THRESHOLD_MS. */
} PressInfo_t;

/**
 * @brief Cumulative statistics updated by Task 2 and read/reset by Task 3.
 *
 * All fields are protected by xSharedDataMutex. Task 2 increments
 * counters after each press; Task 3 reads and resets every 10 seconds.
 */
typedef struct {
    uint32_t totalPresses;    /**< Total button presses since last reset. */
    uint32_t shortPresses;    /**< Count of short presses (< 500 ms). */
    uint32_t longPresses;     /**< Count of long presses (>= 500 ms). */
    uint32_t totalDurationMs; /**< Sum of all press durations in ms. */
} Stats_t;

// ──────────────────────────────────────────────────────────────────────────
// External Declarations
// ──────────────────────────────────────────────────────────────────────────

/** Press event info — written by Task 1, consumed by Task 2. */
extern PressInfo_t g_pressInfo;

/** Accumulated statistics — updated by Task 2, read/reset by Task 3. */
extern Stats_t g_stats;

/**
 * @brief Binary semaphore for event signaling (Task 1 → Task 2).
 *
 * Given by Task 1 when a complete press cycle is detected.
 * Taken by Task 2 to wake up and process the event.
 */
extern SemaphoreHandle_t xNewPressSemaphore;

/**
 * @brief Mutex for protecting shared variables (g_pressInfo, g_stats).
 *
 * Acquired by any task before reading or writing the shared data
 * structures. Provides priority inheritance to prevent priority
 * inversion between Task 1 (high) and Task 3 (low).
 */
extern SemaphoreHandle_t xSharedDataMutex;

/**
 * @brief Create and initialize all synchronization primitives.
 *
 * Must be called once from setup() before creating any FreeRTOS tasks.
 * Creates the binary semaphore and the mutex.
 */
void sharedStateInit();

#endif // SHARED_STATE_H
