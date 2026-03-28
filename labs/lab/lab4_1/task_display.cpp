/**
 * @file task_display.cpp
 * @brief Display and Reporting Task Implementation — Lab 4.1
 *
 * Updates the I2C LCD and generates STDIO reports every 500 ms.
 *
 * LCD line 1 shows the conditioned actuator state (ON / OFF).
 * LCD line 2 shows the signal conditioning status (STABLE / DEBOUNCING).
 *
 * Every STDIO_REPORT_EVERY_N ticks (default 4 × 500 ms = 2 s), a structured
 * text report is printed to the serial terminal with the following fields:
 *   State    — conditioned hardware state
 *   Cmd(raw) — last raw command from the input task
 *   Cond     — conditioning status (STABLE / DEBOUNCING)
 *   Confirm  — current confirm count / required count
 *   Toggles  — total committed state changes since boot
 */

#include "task_display.h"
#include "actuator_data.h"
#include "LcdDisplay.h"
#include <Arduino.h>
#include <stdio.h>

// ── LCD instance ──────────────────────────────────────────────────────────────

static LcdDisplay s_lcd(0x27, 16, 2);

// ── Task implementation ───────────────────────────────────────────────────────

void vTaskDisplay(void *params) {
    (void)params;

    s_lcd.init();
    s_lcd.showTwoLines("Actuator Ctrl", "Initializing...");

    uint8_t tick = 0;

    TickType_t lastWake = xTaskGetTickCount();

    while (true) {
        // ── Read shared state under mutex ─────────────────────────────────
        ActuatorControl::State state    = ActuatorControl::State::OFF;
        ActuatorControl::State rawCmd   = ActuatorControl::State::OFF;
        bool     debouncing  = false;
        uint8_t  confirmCnt  = 0;
        uint32_t toggleCnt   = 0;

        if (xSemaphoreTake(g_actuator.mutex, pdMS_TO_TICKS(10)) == pdTRUE) {
            state       = g_actuator.actuatorState;
            rawCmd      = g_actuator.pendingCmd;
            debouncing  = g_actuator.isDebouncing;
            confirmCnt  = g_actuator.confirmCount;
            toggleCnt   = g_actuator.toggleCount;
            xSemaphoreGive(g_actuator.mutex);
        }

        const char *stateStr = (state  == ActuatorControl::State::ON) ? "ON " : "OFF";
        const char *rawStr   = (rawCmd == ActuatorControl::State::ON) ? "ON " : "OFF";
        const char *condStr  = debouncing ? "DEBOUNCING" : "STABLE    ";

        // ── Update LCD ────────────────────────────────────────────────────
        char line1[17], line2[17];
        snprintf(line1, sizeof(line1), "Actuator: %s   ", stateStr);
        snprintf(line2, sizeof(line2), "%s", condStr);
        s_lcd.showTwoLines(line1, line2);

        // ── Periodic STDIO structured report ──────────────────────────────
        tick++;
        if (tick >= STDIO_REPORT_EVERY_N) {
            tick = 0;
            printf("--[ Actuator Report ]--\r\n");
            printf("  State:    %s\r\n",     stateStr);
            printf("  Cmd(raw): %s\r\n",     rawStr);
            printf("  Cond:     %s\r\n",     condStr);
            printf("  Confirm:  %u/%u\r\n",  (unsigned)confirmCnt,
                                              (unsigned)PERSIST_COUNT);
            printf("  Toggles:  %lu\r\n",    toggleCnt);
            printf("-----------------------\r\n\r\n");
        }

        vTaskDelayUntil(&lastWake, pdMS_TO_TICKS(TASK_DISPLAY_PERIOD_MS));
    }
}
