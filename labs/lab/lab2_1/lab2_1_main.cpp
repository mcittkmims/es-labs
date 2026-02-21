/**
 * @file lab2_1_main.cpp
 * @brief Lab 2.1 — Button Press Duration Monitoring with Non-Preemptive Tasks
 *
 * Implements a three-task cooperative scheduler that monitors button presses,
 * provides visual feedback via coloured LEDs, and periodically reports
 * statistics over the serial STDIO interface.
 *
 * ──────────────────────────────────────────────────────────────────────────
 * System overview
 * ──────────────────────────────────────────────────────────────────────────
 *
 * The application is built on the TaskScheduler library, which provides
 * non-preemptive bare-metal scheduling. Three TaskContext_t entries form a
 * flat array; on each loop() iteration schedulerRun() picks the most-overdue
 * due task and executes it — exactly one task per tick.
 *
 * ┌─────────┬──────────┬────────────────────────────────────────────────┐
 * │ Task    │ Period   │ Responsibility                                 │
 * ├─────────┼──────────┼────────────────────────────────────────────────┤
 * │ Task 1  │  10 ms   │ Button debounce, duration measurement, LED     │
 * │ Task 2  │  50 ms   │ Statistics update, yellow LED blink sequencer  │
 * │ Task 3  │ 10000 ms │ STDIO statistics report + counter reset        │
 * └─────────┴──────────┴────────────────────────────────────────────────┘
 *
 * ──────────────────────────────────────────────────────────────────────────
 * Inter-task communication
 * ──────────────────────────────────────────────────────────────────────────
 *
 * Because the scheduler is non-preemptive (cooperative), shared state is
 * accessed exclusively by one task at a time — no mutexes are needed.
 *
 *   g_newPress          – Set by Task 1 when a complete press cycle is
 *                         detected; cleared by Task 2 after processing.
 *   g_lastPressDuration – Duration (ms) of the most recent press; written
 *                         by Task 1, read by Task 2.
 *   g_isShortPress      – TRUE if duration < SHORT_PRESS_THRESHOLD_MS.
 *   g_totalPresses      – Cumulative press count (reset by Task 3).
 *   g_shortPresses      – Short-press count (reset by Task 3).
 *   g_longPresses       – Long-press count (reset by Task 3).
 *   g_totalDurationMs   – Sum of all press durations in ms (reset by Task 3).
 */

#include "lab2_1_main.h"

#include <Arduino.h>
#include <stdio.h>

#include "TaskScheduler.h"
#include "StdioSerial.h"

// ──────────────────────────────────────────────────────────────────────────
// Hardware pin mapping
// ──────────────────────────────────────────────────────────────────────────

static const uint8_t PIN_BUTTON     = 7;   /**< Push button (active-LOW, INPUT_PULLUP) */
static const uint8_t PIN_LED_GREEN  = 8;   /**< Green LED — short press (< 500 ms)     */
static const uint8_t PIN_LED_RED    = 9;   /**< Red LED   — long press  (≥ 500 ms)     */
static const uint8_t PIN_LED_YELLOW = 10;  /**< Yellow LED — activity blink sequencer  */

// ──────────────────────────────────────────────────────────────────────────
// Application constants
// ──────────────────────────────────────────────────────────────────────────

/** A press shorter than this value (ms) is classified as "short". */
static const uint32_t SHORT_PRESS_THRESHOLD_MS = 500;

/** How long the green/red LED stays on after a completed press (ms). */
static const uint32_t LED_INDICATOR_DURATION_MS = 1500;

/** Half-period for one yellow LED blink step (ms). On=100 ms, Off=100 ms. */
static const uint32_t BLINK_HALF_PERIOD_MS = 100;

/** Number of yellow LED half-cycles (ON+OFF = 2 steps) per blink count. */
static const uint8_t BLINK_STEPS_SHORT = 10;   /**< 5 blinks × 2 half-cycles */
static const uint8_t BLINK_STEPS_LONG  = 20;   /**< 10 blinks × 2 half-cycles */

// ──────────────────────────────────────────────────────────────────────────
// Shared global state (inter-task communication)
// ──────────────────────────────────────────────────────────────────────────

/** Set to true by Task 1 when a complete press is available; cleared by Task 2. */
static volatile bool     g_newPress          = false;

/** Duration of the most recent completed press, in milliseconds. */
static volatile uint32_t g_lastPressDuration  = 0;

/** True if the most recent press was a short press (< SHORT_PRESS_THRESHOLD_MS). */
static volatile bool     g_isShortPress      = false;

/** Total number of button presses since last Task 3 reset. */
static volatile uint32_t g_totalPresses      = 0;

/** Number of short presses since last Task 3 reset. */
static volatile uint32_t g_shortPresses      = 0;

/** Number of long presses since last Task 3 reset. */
static volatile uint32_t g_longPresses       = 0;

/** Accumulated press duration in milliseconds since last Task 3 reset. */
static volatile uint32_t g_totalDurationMs   = 0;

// ──────────────────────────────────────────────────────────────────────────
// Task 1 — private state
// ──────────────────────────────────────────────────────────────────────────

/** Button finite state machine states. */
typedef enum {
    BTN_IDLE,           /**< No button activity detected.                        */
    BTN_DEBOUNCE_DOWN,  /**< Button went LOW; waiting for debounce confirmation.  */
    BTN_PRESSED,        /**< Button confirmed pressed; measuring duration.        */
    BTN_DEBOUNCE_UP     /**< Button went HIGH; waiting for debounce confirmation. */
} ButtonState_e;

static ButtonState_e s_btnState      = BTN_IDLE;
static uint32_t      s_debounceStart = 0;  /**< millis() when debounce began.         */
static uint32_t      s_pressStart    = 0;  /**< millis() when press was confirmed.    */
static uint32_t      s_pressEnd      = 0;  /**< millis() when release was detected.   */

/** Absolute time (ms) when the green LED should be turned off. 0 = off. */
static uint32_t s_greenLedOffAt = 0;

/** Absolute time (ms) when the red LED should be turned off. 0 = off. */
static uint32_t s_redLedOffAt   = 0;

/** Debounce duration: button must stay in new state for this many ms. */
static const uint32_t DEBOUNCE_MS = 50;

// ──────────────────────────────────────────────────────────────────────────
// Task 2 — private state (yellow LED blink sequencer)
// ──────────────────────────────────────────────────────────────────────────

/** Remaining half-cycle steps for the yellow LED blink sequence. 0 = idle. */
static uint8_t  s_blinkStepsRemaining = 0;

/** millis() timestamp of the last yellow LED toggle. */
static uint32_t s_blinkLastToggle     = 0;

/** Current logical state of the yellow LED (true = ON). */
static bool     s_yellowLedOn        = false;

// ──────────────────────────────────────────────────────────────────────────
// Task function prototypes
// ──────────────────────────────────────────────────────────────────────────

static void task1ButtonAndLed();
static void task2StatisticsAndBlink();
static void task3PeriodicReport();

// ──────────────────────────────────────────────────────────────────────────
// Task context array (recurrence table)
// ──────────────────────────────────────────────────────────────────────────

/**
 * Task context array — the central scheduling table.
 *
 * Each entry holds: { funcPtr, period_ms, offset_ms, nextRun (filled by init) }.
 * The offset staggers startup to avoid all three tasks firing simultaneously
 * on the very first scheduler ticks.
 */
static TaskContext_t s_tasks[] = {
    { task1ButtonAndLed,        10,     0  },  /**< Task 1: button + LED, 10 ms */
    { task2StatisticsAndBlink,  50,     5  },  /**< Task 2: stats + blink, 50 ms */
    { task3PeriodicReport,   10000,  2000  },  /**< Task 3: report, 10 s (first after 2 s) */
};

static const uint8_t TASK_COUNT = sizeof(s_tasks) / sizeof(s_tasks[0]);

// ──────────────────────────────────────────────────────────────────────────
// Task 1 — Button Detection, Duration Measurement, Indicator LEDs
// ──────────────────────────────────────────────────────────────────────────

/**
 * @brief Task 1 body — runs every 10 ms.
 *
 * Drives a four-state FSM to debounce the button and measure the duration
 * between the confirmed press edge and the confirmed release edge. On each
 * completed press:
 *   - Records duration and type in the shared global flags.
 *   - Turns on the green LED (short press) or red LED (long press) for
 *     LED_INDICATOR_DURATION_MS milliseconds.
 *   - Manages automatic LED turn-off based on a one-shot timer.
 *
 * FSM transitions:
 *   IDLE           → DEBOUNCE_DOWN : button read LOW
 *   DEBOUNCE_DOWN  → PRESSED       : still LOW after DEBOUNCE_MS
 *   DEBOUNCE_DOWN  → IDLE          : went HIGH before debounce expired
 *   PRESSED        → DEBOUNCE_UP   : button read HIGH
 *   DEBOUNCE_UP    → IDLE          : still HIGH after DEBOUNCE_MS (press complete)
 *   DEBOUNCE_UP    → PRESSED       : went LOW before debounce expired
 */
static void task1ButtonAndLed() {
    uint32_t now       = millis();
    bool     btnLow    = (digitalRead(PIN_BUTTON) == LOW);

    // ── FSM update ─────────────────────────────────────────────────────
    switch (s_btnState) {

        case BTN_IDLE:
            if (btnLow) {
                s_debounceStart = now;
                s_btnState      = BTN_DEBOUNCE_DOWN;
            }
            break;

        case BTN_DEBOUNCE_DOWN:
            if (!btnLow) {
                // Spurious glitch — button released before debounce elapsed.
                s_btnState = BTN_IDLE;
            } else if ((now - s_debounceStart) >= DEBOUNCE_MS) {
                // Confirmed press.
                s_pressStart = now;
                s_btnState   = BTN_PRESSED;
            }
            break;

        case BTN_PRESSED:
            if (!btnLow) {
                // Button released — start release debounce.
                s_pressEnd      = now;
                s_debounceStart = now;
                s_btnState      = BTN_DEBOUNCE_UP;
            }
            break;

        case BTN_DEBOUNCE_UP:
            if (btnLow) {
                // Button went back down before debounce expired — still pressed.
                s_btnState = BTN_PRESSED;
            } else if ((now - s_debounceStart) >= DEBOUNCE_MS) {
                // Confirmed release — compute duration and signal result.
                uint32_t duration = s_pressEnd - s_pressStart;

                g_lastPressDuration = duration;
                g_isShortPress      = (duration < SHORT_PRESS_THRESHOLD_MS);
                g_newPress          = true;   // Signal Task 2

                // Light the appropriate indicator LED.
                if (g_isShortPress) {
                    digitalWrite(PIN_LED_GREEN, HIGH);
                    s_greenLedOffAt = now + LED_INDICATOR_DURATION_MS;
                } else {
                    digitalWrite(PIN_LED_RED, HIGH);
                    s_redLedOffAt = now + LED_INDICATOR_DURATION_MS;
                }

                s_btnState = BTN_IDLE;
            }
            break;
    }

    // ── Auto-off for indicator LEDs ────────────────────────────────────
    if (s_greenLedOffAt != 0 && now >= s_greenLedOffAt) {
        digitalWrite(PIN_LED_GREEN, LOW);
        s_greenLedOffAt = 0;
    }
    if (s_redLedOffAt != 0 && now >= s_redLedOffAt) {
        digitalWrite(PIN_LED_RED, LOW);
        s_redLedOffAt = 0;
    }
}

// ──────────────────────────────────────────────────────────────────────────
// Task 2 — Statistics Update & Yellow LED Blink Sequencer
// ──────────────────────────────────────────────────────────────────────────

/**
 * @brief Task 2 body — runs every 50 ms.
 *
 * Checks the g_newPress flag set by Task 1. When a new press is available:
 *   - Increments global press counters and duration accumulator.
 *   - Arms the yellow LED blink sequencer: 5 blinks (10 half-cycles) for a
 *     short press, 10 blinks (20 half-cycles) for a long press.
 *   - Clears the g_newPress flag.
 *
 * Independently of new presses, the blink sequencer advances each time
 * BLINK_HALF_PERIOD_MS has elapsed since the last toggle, driving the
 * yellow LED on and off until the sequence is complete.
 */
static void task2StatisticsAndBlink() {
    uint32_t now = millis();

    // ── Consume new press event ─────────────────────────────────────────
    if (g_newPress) {
        g_newPress = false;

        // Update statistics.
        g_totalPresses++;
        g_totalDurationMs += g_lastPressDuration;

        if (g_isShortPress) {
            g_shortPresses++;
            s_blinkStepsRemaining = BLINK_STEPS_SHORT;
        } else {
            g_longPresses++;
            s_blinkStepsRemaining = BLINK_STEPS_LONG;
        }

        // Start the blink sequence: turn LED on immediately.
        digitalWrite(PIN_LED_YELLOW, HIGH);
        s_yellowLedOn    = true;
        s_blinkLastToggle = now;
    }

    // ── Advance blink sequencer ─────────────────────────────────────────
    if (s_blinkStepsRemaining > 0) {
        if ((now - s_blinkLastToggle) >= BLINK_HALF_PERIOD_MS) {
            // Toggle yellow LED.
            s_yellowLedOn = !s_yellowLedOn;
            digitalWrite(PIN_LED_YELLOW, s_yellowLedOn ? HIGH : LOW);
            s_blinkLastToggle = now;
            s_blinkStepsRemaining--;

            // Ensure LED is OFF when sequence ends.
            if (s_blinkStepsRemaining == 0) {
                digitalWrite(PIN_LED_YELLOW, LOW);
                s_yellowLedOn = false;
            }
        }
    }
}

// ──────────────────────────────────────────────────────────────────────────
// Task 3 — Periodic STDIO Statistics Report
// ──────────────────────────────────────────────────────────────────────────

/**
 * @brief Task 3 body — runs every 10 000 ms.
 *
 * Reads the current statistics captured by Task 2, computes the average press
 * duration, and prints a formatted report to the STDIO serial terminal. After
 * printing, all accumulators are reset to begin a fresh 10-second window.
 *
 * Output format:
 *   ===== [Report] =====
 *   Total presses : <n>
 *   Short presses : <n>
 *   Long presses  : <n>
 *   Average duration: <n> ms
 *   ====================
 */
static void task3PeriodicReport() {
    uint32_t total    = g_totalPresses;
    uint32_t shorts   = g_shortPresses;
    uint32_t longs    = g_longPresses;
    uint32_t totalMs  = g_totalDurationMs;

    uint32_t avgMs = (total > 0) ? (totalMs / total) : 0;

    printf("\r\n===== [10s Report] =====\r\n");
    printf("Total presses    : %lu\r\n", total);
    printf("Short presses    : %lu  (< %u ms)\r\n", shorts, (unsigned)SHORT_PRESS_THRESHOLD_MS);
    printf("Long presses     : %lu  (>= %u ms)\r\n", longs, (unsigned)SHORT_PRESS_THRESHOLD_MS);
    printf("Average duration : %lu ms\r\n", avgMs);
    printf("========================\r\n");

    // Reset accumulators for the next reporting window.
    g_totalPresses    = 0;
    g_shortPresses    = 0;
    g_longPresses     = 0;
    g_totalDurationMs = 0;
}

// ──────────────────────────────────────────────────────────────────────────
// Lab 2.1 public entry points
// ──────────────────────────────────────────────────────────────────────────

void lab2_1Setup() {
    // Configure hardware pins.
    pinMode(PIN_BUTTON,     INPUT_PULLUP);
    pinMode(PIN_LED_GREEN,  OUTPUT);
    pinMode(PIN_LED_RED,    OUTPUT);
    pinMode(PIN_LED_YELLOW, OUTPUT);

    // Ensure all LEDs start in the OFF state.
    digitalWrite(PIN_LED_GREEN,  LOW);
    digitalWrite(PIN_LED_RED,    LOW);
    digitalWrite(PIN_LED_YELLOW, LOW);

    // Initialize STDIO serial at 9600 baud.
    stdioSerialInit(9600);

    // Print startup banner.
    printf("\r\n");
    printf("========================================\r\n");
    printf("  Lab 2.1 — Button Press Monitor        \r\n");
    printf("  Non-Preemptive Task Scheduler Demo    \r\n");
    printf("  Tasks: 3 | Tick base: 10 ms           \r\n");
    printf("========================================\r\n");
    printf("GREEN  LED  = short press (< %u ms)\r\n",   (unsigned)SHORT_PRESS_THRESHOLD_MS);
    printf("RED    LED  = long press  (>= %u ms)\r\n",  (unsigned)SHORT_PRESS_THRESHOLD_MS);
    printf("YELLOW LED  = activity blink\r\n");
    printf("Report interval: 10 seconds\r\n");
    printf("========================================\r\n\r\n");

    // Initialize the scheduler (sets nextRun = millis() + offset for each task).
    schedulerInit(s_tasks, TASK_COUNT);
}

void lab2_1Loop() {
    // Each call to schedulerRun() executes at most one due task.
    schedulerRun(s_tasks, TASK_COUNT);
}
