/**
 * @file lab4_1_main.cpp
 * @brief Lab 4.1 — Dual Actuator Control System (Varianta C)
 *
 * Implements a dual-actuator control system with FreeRTOS:
 *   - Binary actuator (relay) controlled via ON/OFF serial commands
 *   - Analog actuator (PWM) controlled via SPEED 0-100 serial commands
 *   - Signal conditioning: debouncing for binary, ramping+filtering for analog
 *   - LCD + serial reporting of both actuator states and alerts
 *
 * FreeRTOS Tasks:
 *   1. commandTask   (50ms)  — reads and parses serial commands
 *   2. binaryTask    (100ms) — debounces binary command, drives relay + LED
 *   3. analogTask    (100ms) — conditions analog setpoint, drives PWM + LED
 *   4. displayTask   (500ms) — updates LCD and prints serial report
 *
 * Inter-task communication uses shared variables protected by a FreeRTOS
 * mutex to prevent data races between command parsing and actuator tasks.
 */

#include <Arduino.h>
#include <Arduino_FreeRTOS.h>
#include <semphr.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>

#include "lab4_1_main.h"
#include "lab4_1_config.h"
#include "Relay.h"
#include "PwmActuator.h"
#include "BinaryConditioner.h"
#include "SignalConditioner.h"
#include "LcdDisplay.h"
#include "Led.h"
#include "StdioSerial.h"

// ── Hardware Instances ──────────────────────────────────────────────────
static Relay         relay(PIN_RELAY, true);
static PwmActuator   pwmAct(PIN_PWM_ACTUATOR);
static Led           ledBinary(PIN_LED_BINARY);
static Led           ledAnalog(PIN_LED_ANALOG);
static LcdDisplay    lcd(LCD_I2C_ADDR, LCD_COLS, LCD_ROWS);

// ── Signal Conditioners ─────────────────────────────────────────────────
static BinaryConditioner binCond(BINARY_DEBOUNCE_COUNT);
static SignalConditioner anlCond(ANALOG_MEDIAN_WINDOW, ANALOG_EWMA_ALPHA,
                                 ANALOG_MIN_CLAMP, ANALOG_MAX_CLAMP);

// ── Shared State (protected by mutex) ───────────────────────────────────
static SemaphoreHandle_t stateMutex;

// Command targets (written by commandTask, read by actuator tasks)
static volatile bool  cmdBinaryTarget  = false;  // Desired relay state
static volatile float cmdAnalogTarget  = 0.0f;   // Desired PWM duty (0-100%)

// Actuator outputs (written by actuator tasks, read by displayTask)
static volatile bool  actBinaryState   = false;  // Current relay state
static volatile float actAnalogDuty    = 0.0f;   // Current PWM duty %
static volatile float actAnalogRaw     = 0.0f;   // Raw command before conditioning

// Conditioner diagnostics
static volatile uint8_t binDebounceCounter = 0;
static volatile bool    binIsTransitioning = false;

// ── Command Parsing Helpers ─────────────────────────────────────────────

/**
 * @brief Trim leading whitespace from a string in place.
 */
static void trimLeading(char *s) {
    char *p = s;
    while (*p && isspace((unsigned char)*p)) p++;
    if (p != s) memmove(s, p, strlen(p) + 1);
}

/**
 * @brief Convert a string to uppercase in place.
 */
static void toUpper(char *s) {
    for (; *s; s++) *s = toupper((unsigned char)*s);
}

/**
 * @brief Parse a serial command and update shared targets.
 *
 * Supported commands:
 *   ON       — turn binary actuator ON
 *   OFF      — turn binary actuator OFF
 *   SPEED N  — set analog actuator to N% (0-100)
 *   STATUS   — print current state (handled in-line)
 */
static void parseAndApplyCommand(char *cmd) {
    trimLeading(cmd);
    // Strip trailing newline/CR
    size_t len = strlen(cmd);
    while (len > 0 && (cmd[len - 1] == '\n' || cmd[len - 1] == '\r')) {
        cmd[--len] = '\0';
    }
    if (len == 0) return;

    // Work on uppercase copy
    char upper[32];
    strncpy(upper, cmd, sizeof(upper) - 1);
    upper[sizeof(upper) - 1] = '\0';
    toUpper(upper);

    if (strcmp(upper, "ON") == 0) {
        xSemaphoreTake(stateMutex, portMAX_DELAY);
        cmdBinaryTarget = true;
        xSemaphoreGive(stateMutex);
        printf("[CMD] Binary actuator -> ON\r\n");

    } else if (strcmp(upper, "OFF") == 0) {
        xSemaphoreTake(stateMutex, portMAX_DELAY);
        cmdBinaryTarget = false;
        xSemaphoreGive(stateMutex);
        printf("[CMD] Binary actuator -> OFF\r\n");

    } else if (strncmp(upper, "SPEED ", 6) == 0) {
        float val = atof(cmd + 6);
        xSemaphoreTake(stateMutex, portMAX_DELAY);
        cmdAnalogTarget = val;
        xSemaphoreGive(stateMutex);
        printf("[CMD] Analog actuator -> %.1f%%\r\n", (double)val);

    } else if (strcmp(upper, "STATUS") == 0) {
        // Read state under mutex
        xSemaphoreTake(stateMutex, portMAX_DELAY);
        bool    bState = actBinaryState;
        float   aDuty  = actAnalogDuty;
        xSemaphoreGive(stateMutex);
        printf("[STATUS] Relay: %s | PWM: %.1f%%\r\n",
               bState ? "ON" : "OFF", (double)aDuty);

    } else {
        printf("[CMD] Unknown command: '%s'\r\n", cmd);
        printf("  Commands: ON, OFF, SPEED <0-100>, STATUS\r\n");
    }
}

// ── FreeRTOS Task Functions ─────────────────────────────────────────────

/**
 * @brief Command Task — reads serial input and parses commands.
 * Runs every TASK_COMMAND_PERIOD_MS (50 ms).
 */
static void commandTask(void *pvParameters) {
    (void)pvParameters;
    static char buf[32];
    static uint8_t idx = 0;

    for (;;) {
        // Read all available serial characters
        while (Serial.available() > 0) {
            char c = (char)Serial.read();
            // Echo character back
            Serial.write(c);

            if (c == '\n' || c == '\r') {
                if (idx > 0) {
                    buf[idx] = '\0';
                    parseAndApplyCommand(buf);
                    idx = 0;
                }
            } else if (idx < sizeof(buf) - 1) {
                buf[idx++] = c;
            }
        }
        vTaskDelay(pdMS_TO_TICKS(TASK_COMMAND_PERIOD_MS));
    }
}

/**
 * @brief Binary Actuator Task — applies debouncing and drives relay.
 * Runs every TASK_BINARY_PERIOD_MS (100 ms).
 */
static void binaryTask(void *pvParameters) {
    (void)pvParameters;

    for (;;) {
        // Read target under mutex
        xSemaphoreTake(stateMutex, portMAX_DELAY);
        bool target = cmdBinaryTarget;
        xSemaphoreGive(stateMutex);

        // Apply debounce conditioning
        bool stableState = binCond.process(target);

        // Drive relay and status LED
        relay.setState(stableState);
        if (stableState) {
            ledBinary.turnOn();
        } else {
            ledBinary.turnOff();
        }

        // Update shared diagnostics
        xSemaphoreTake(stateMutex, portMAX_DELAY);
        actBinaryState     = stableState;
        binDebounceCounter = binCond.getCounter();
        binIsTransitioning = binCond.isTransitioning();
        xSemaphoreGive(stateMutex);

        vTaskDelay(pdMS_TO_TICKS(TASK_BINARY_PERIOD_MS));
    }
}

/**
 * @brief Analog Actuator Task — conditions setpoint with ramping and drives PWM.
 * Runs every TASK_ANALOG_PERIOD_MS (100 ms).
 *
 * The ramping logic limits the rate of change per cycle to ANALOG_RAMP_RATE,
 * preventing sudden jumps that could damage mechanical actuators. The ramped
 * value is then fed through the SignalConditioner pipeline (saturation +
 * median + EWMA) for additional smoothing.
 */
static void analogTask(void *pvParameters) {
    (void)pvParameters;
    static float currentSetpoint = 0.0f;

    for (;;) {
        // Read target under mutex
        xSemaphoreTake(stateMutex, portMAX_DELAY);
        float target = cmdAnalogTarget;
        xSemaphoreGive(stateMutex);

        // Apply ramping: limit rate of change per cycle
        float diff = target - currentSetpoint;
        if (diff > ANALOG_RAMP_RATE) {
            currentSetpoint += ANALOG_RAMP_RATE;
        } else if (diff < -ANALOG_RAMP_RATE) {
            currentSetpoint -= ANALOG_RAMP_RATE;
        } else {
            currentSetpoint = target;
        }

        // Feed through signal conditioning pipeline
        float conditioned = anlCond.process(currentSetpoint);

        // Drive PWM actuator
        pwmAct.setDutyCycle(conditioned);

        // Update analog status LED brightness (approximate with on/off threshold)
        if (conditioned > 1.0f) {
            ledAnalog.turnOn();
        } else {
            ledAnalog.turnOff();
        }

        // Update shared state
        xSemaphoreTake(stateMutex, portMAX_DELAY);
        actAnalogDuty = conditioned;
        actAnalogRaw  = currentSetpoint;
        xSemaphoreGive(stateMutex);

        vTaskDelay(pdMS_TO_TICKS(TASK_ANALOG_PERIOD_MS));
    }
}

/**
 * @brief Display Task — updates LCD and prints serial report.
 * Runs every TASK_DISPLAY_PERIOD_MS (500 ms).
 *
 * LCD layout:
 *   Row 0: "Rly:ON  PWM: 75%"
 *   Row 1: "Deb:5/5 Raw:75.0"
 */
static void displayTask(void *pvParameters) {
    (void)pvParameters;
    char line0[17];
    char line1[17];

    for (;;) {
        // Read all state under mutex
        xSemaphoreTake(stateMutex, portMAX_DELAY);
        bool    bState   = actBinaryState;
        float   aDuty    = actAnalogDuty;
        float   aRaw     = actAnalogRaw;
        uint8_t debCnt   = binDebounceCounter;
        bool    debTrans = binIsTransitioning;
        xSemaphoreGive(stateMutex);

        // Format LCD lines
        snprintf(line0, sizeof(line0), "Rly:%-3s PWM:%3d%%",
                 bState ? "ON" : "OFF", (int)aDuty);
        snprintf(line1, sizeof(line1), "Deb:%d/%d Raw:%3d%%",
                 debCnt, BINARY_DEBOUNCE_COUNT, (int)aRaw);

        lcd.showTwoLines(line0, line1);

        // Serial report
        printf("--- Actuator Report ---\r\n");
        printf("  Binary: %s (debounce: %d/%d%s)\r\n",
               bState ? "ON" : "OFF", debCnt, BINARY_DEBOUNCE_COUNT,
               debTrans ? " TRANSITIONING" : "");
        printf("  Analog: duty=%.1f%% raw=%.1f%% pwm=%d/255\r\n",
               (double)aDuty, (double)aRaw, (int)(aDuty * 255.0f / 100.0f));
        printf("-----------------------\r\n");

        vTaskDelay(pdMS_TO_TICKS(TASK_DISPLAY_PERIOD_MS));
    }
}

// ── Setup & Loop ────────────────────────────────────────────────────────

void lab4_1Setup() {
    // Initialize serial with STDIO redirection
    stdioSerialInit(SERIAL_BAUD);

    printf("\r\n=== Lab 4.1: Dual Actuator Control (Varianta C) ===\r\n");
    printf("Commands: ON, OFF, SPEED <0-100>, STATUS\r\n\r\n");

    // Initialize hardware
    relay.init();
    pwmAct.init();
    ledBinary.init();
    ledAnalog.init();
    lcd.init();
    lcd.showTwoLines("Lab 4.1: Dual", "Actuator Ctrl");

    // Initialize conditioners
    binCond.init();
    anlCond.reset();

    // Create mutex
    stateMutex = xSemaphoreCreateMutex();

    // Create FreeRTOS tasks
    xTaskCreate(commandTask, "CmdTask",  STACK_COMMAND, NULL, 3, NULL);
    xTaskCreate(binaryTask,  "BinTask",  STACK_BINARY,  NULL, 2, NULL);
    xTaskCreate(analogTask,  "AnlTask",  STACK_ANALOG,  NULL, 2, NULL);
    xTaskCreate(displayTask, "DispTask", STACK_DISPLAY,  NULL, 1, NULL);

    // FreeRTOS scheduler starts automatically after setup() returns
}

void lab4_1Loop() {
    // Empty — FreeRTOS scheduler manages all tasks
}
