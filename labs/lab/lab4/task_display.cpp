/**
 * @file task_display.cpp
 * @brief Lab 4 — Display & Reporting Task Implementation
 *
 * Updates the LCD with a two-line summary of both actuators:
 *   Line 1: "Relay:ON  PWM:75%"
 *   Line 2: "Ramp:72% ALR:NO"
 *
 * Every 2 seconds, prints a structured report to the serial terminal
 * with full pipeline diagnostics (raw, conditioned, ramped, alert).
 */

#include "task_display.h"
#include "shared_state.h"
#include "lab4_config.h"

#include "LcdDisplay.h"
#include <stdio.h>
#include <stdlib.h>  // dtostrf

static LcdDisplay lcd(LCD_I2C_ADDR, LCD_COLS, LCD_ROWS);

void vTaskDisplay(void *pvParameters) {
    (void)pvParameters;
    lcd.init();

    TickType_t xLastWake = xTaskGetTickCount();
    const TickType_t xPeriod = pdMS_TO_TICKS(TASK_DISPLAY_PERIOD_MS);

    uint8_t reportCounter = 0;

    for (;;) {
        sharedStateLock();
        ActuatorState *s = sharedStateGet();

        // Snapshot values while locked
        bool relayOn = s->relayActualOn;
        float pwmCmd = s->pwmCommandPercent;
        float pwmCond = s->pwmConditioned;
        float pwmRamp = s->pwmRamped;
        uint8_t pwmRaw = s->pwmRawValue;
        bool alert = s->overloadAlert;
        bool inputMode = s->inputModeAnalog;
        char inputBuf[4];
        uint8_t inputLen = s->inputBufferLen;
        for (uint8_t i = 0; i < inputLen && i < 3; i++)
            inputBuf[i] = s->inputBuffer[i];
        inputBuf[inputLen] = '\0';

        sharedStateUnlock();

        // ── LCD update ──────────────────────────────────────────────
        char line1[17], line2[17];

        if (inputMode) {
            snprintf(line1, 17, "Relay:%-3s [EDIT]",
                     relayOn ? "ON" : "OFF");
            snprintf(line2, 17, "PWM=%-3s%%  *=CLR", inputBuf);
        } else {
            int rampInt = (int)(pwmRamp + 0.5f);
            snprintf(line1, 17, "Relay:%-3s PWM%3d%%",
                     relayOn ? "ON" : "OFF", (int)(pwmCmd + 0.5f));
            snprintf(line2, 17, "Out:%3d%% %s",
                     rampInt, alert ? "!ALERT" : "  OK  ");
        }

        lcd.showTwoLines(line1, line2);

        // ── Serial report every 2 seconds (4 x 500ms) ──────────────
        reportCounter++;
        if (reportCounter >= 4) {
            reportCounter = 0;

            char cmdBuf[8], condBuf[8], rampBuf[8];
            dtostrf(pwmCmd, 5, 1, cmdBuf);
            dtostrf(pwmCond, 5, 1, condBuf);
            dtostrf(pwmRamp, 5, 1, rampBuf);

            printf("\r\n--- Actuator Status Report ---\r\n");
            printf("BINARY ACTUATOR (Relay):\r\n");
            printf("  State: %s\r\n", relayOn ? "ON" : "OFF");
            printf("ANALOG ACTUATOR (PWM):\r\n");
            printf("  Command:      %s %%\r\n", cmdBuf);
            printf("  Conditioned:  %s %%\r\n", condBuf);
            printf("  Ramped (out): %s %%\r\n", rampBuf);
            printf("  PWM register: %u / 255\r\n", (unsigned)pwmRaw);
            printf("ALERT: %s\r\n", alert ? "OVERLOAD ACTIVE" : "Normal");
            printf("------------------------------\r\n");
        }

        vTaskDelayUntil(&xLastWake, xPeriod);
    }
}
