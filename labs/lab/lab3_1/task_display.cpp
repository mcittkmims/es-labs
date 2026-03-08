/**
 * @file task_display.cpp
 * @brief Lab 3.1 — Display & Reporting Task Implementation (Task 3)
 *
 * Implements the periodic display and reporting task. Every 500 ms,
 * this task reads sensor data and alert status, updates the LCD with
 * a summary, and prints a structured report to the serial terminal
 * via STDIO printf.
 *
 * ──────────────────────────────────────────────────────────────────────────
 * Display strategy
 * ──────────────────────────────────────────────────────────────────────────
 *
 * The 16x2 LCD alternates between two information pages every 2 seconds:
 *
 *   Page 0 — Current readings:
 *     Line 0: "A:25.3C  D:24.8C"   (analog and digital temperatures)
 *     Line 1: "OK" or "ALERT:A" or "ALERT:D" or "ALERT:A+D"
 *
 *   Page 1 — Threshold configuration:
 *     Line 0: "AH:30 AL:28 C"      (analog high/low thresholds)
 *     Line 1: "DH:30 DL:28 C"      (digital high/low thresholds)
 *
 * ──────────────────────────────────────────────────────────────────────────
 * STDIO report format (every 2 seconds = every 4th display cycle)
 * ──────────────────────────────────────────────────────────────────────────
 *
 *   ┌─────────────────────────────────────────────────┐
 *   │ === SENSOR REPORT #NNN ===                      │
 *   │ Analog:  25.3°C (raw=512, R=10234Ω) [NORMAL]   │
 *   │ Digital: 24.8°C                      [NORMAL]   │
 *   │ Alerts: A=0 D=0  Cycles: 1234                   │
 *   └─────────────────────────────────────────────────┘
 */

#include "task_display.h"
#include "sensor_data.h"

#include "LcdDisplay.h"
#include <stdio.h>
#include <string.h>
#include <math.h>

// ──────────────────────────────────────────────────────────────────────────
// Local LCD instance (owned by this task)
// ──────────────────────────────────────────────────────────────────────────

static LcdDisplay s_lcd(LCD_I2C_ADDRESS, LCD_COLS, LCD_ROWS);

// ──────────────────────────────────────────────────────────────────────────
// Helper: format a float temperature for LCD (5 chars: "XX.X" or "-X.X")
// ──────────────────────────────────────────────────────────────────────────

static void formatTemp(char *buf, size_t bufLen, float temp) {
    if (isnan(temp)) {
        strncpy(buf, " -- ", bufLen);
    } else {
        dtostrf(temp, 4, 1, buf);
    }
}

// ──────────────────────────────────────────────────────────────────────────
// Helper: get short alert state label
// ──────────────────────────────────────────────────────────────────────────

static const char* alertShortLabel(AlertState state) {
    switch (state) {
        case ALERT_NORMAL:         return "OK";
        case ALERT_DEBOUNCE_HIGH:  return "DB+";
        case ALERT_ACTIVE:         return "ALR";
        case ALERT_DEBOUNCE_LOW:   return "DB-";
        default:                   return "???";
    }
}

static const char* alertFullLabel(AlertState state) {
    switch (state) {
        case ALERT_NORMAL:         return "NORMAL";
        case ALERT_DEBOUNCE_HIGH:  return "DEBOUNCE_HIGH";
        case ALERT_ACTIVE:         return "ALERT";
        case ALERT_DEBOUNCE_LOW:   return "DEBOUNCE_LOW";
        default:                   return "UNKNOWN";
    }
}

// ──────────────────────────────────────────────────────────────────────────
// Task function
// ──────────────────────────────────────────────────────────────────────────

void vTaskDisplay(void *pvParameters) {
    (void)pvParameters;

    // Initialize the LCD display.
    s_lcd.init();
    s_lcd.backlight(true);
    s_lcd.showTwoLines("Lab 3.1 Sensors", "Initializing...");

    // Allow sensors to stabilize before first report.
    vTaskDelay(pdMS_TO_TICKS(1000));

    TickType_t xLastWakeTime = xTaskGetTickCount();
    const TickType_t xPeriod = pdMS_TO_TICKS(TASK_DISPLAY_PERIOD_MS);

    // LCD page alternation counter (switch every 4 cycles = 2 seconds).
    uint8_t displayCycle = 0;
    uint32_t reportNumber = 0;

    // STDIO report interval: every 4 display cycles (2 seconds).
    static const uint8_t REPORT_INTERVAL = 4;

    // Local copies of shared data.
    SensorReadings_t localSensor;
    AlertStatus_t    localAlert;

    char line0[17];  // LCD line buffer (16 chars + null)
    char line1[17];

    for (;;) {
        vTaskDelayUntil(&xLastWakeTime, xPeriod);
        displayCycle++;

        // ── Read shared data under mutex ──────────────────────────────
        if (xSemaphoreTake(xSensorMutex, pdMS_TO_TICKS(20)) == pdTRUE) {
            memcpy(&localSensor, &g_sensorData, sizeof(SensorReadings_t));
            memcpy(&localAlert,  &g_alertData,  sizeof(AlertStatus_t));
            xSemaphoreGive(xSensorMutex);
        } else {
            continue;  // Skip this cycle if mutex unavailable.
        }

        // ── Update LCD display ────────────────────────────────────────
        uint8_t page = (displayCycle / 4) % 2;

        if (page == 0) {
            // Page 0: Current sensor readings and alert status.
            char aTemp[8], dTemp[8];
            formatTemp(aTemp, sizeof(aTemp), localSensor.analogTempC);
            formatTemp(dTemp, sizeof(dTemp), localSensor.digitalTempC);

            snprintf(line0, sizeof(line0), "A:%sC D:%sC", aTemp, dTemp);

            bool aAlert = (localAlert.analogAlertState == ALERT_ACTIVE);
            bool dAlert = (localAlert.digitalAlertState == ALERT_ACTIVE);

            if (aAlert && dAlert) {
                snprintf(line1, sizeof(line1), "ALERT: A+D");
            } else if (aAlert) {
                snprintf(line1, sizeof(line1), "ALERT: Analog");
            } else if (dAlert) {
                snprintf(line1, sizeof(line1), "ALERT: Digital");
            } else {
                snprintf(line1, sizeof(line1), "System: OK");
            }
        } else {
            // Page 1: Threshold settings.
            // AVR printf/snprintf does NOT support %f — use dtostrf.
            char ahStr[6], alStr[6], dhStr[6], dlStr[6];
            dtostrf(ANALOG_THRESHOLD_HIGH,  3, 0, ahStr);
            dtostrf(ANALOG_THRESHOLD_LOW,   3, 0, alStr);
            dtostrf(DIGITAL_THRESHOLD_HIGH, 3, 0, dhStr);
            dtostrf(DIGITAL_THRESHOLD_LOW,  3, 0, dlStr);
            snprintf(line0, sizeof(line0), "AH:%s AL:%s C", ahStr, alStr);
            snprintf(line1, sizeof(line1), "DH:%s DL:%s C", dhStr, dlStr);
        }

        s_lcd.showTwoLines(line0, line1);

        // ── Structured STDIO report (every 2 seconds) ────────────────
        if ((displayCycle % REPORT_INTERVAL) == 0) {
            reportNumber++;

            char aTempStr[8], dTempStr[8];
            formatTemp(aTempStr, sizeof(aTempStr), localSensor.analogTempC);
            formatTemp(dTempStr, sizeof(dTempStr), localSensor.digitalTempC);

            printf("\r\n");
            printf("====== SENSOR REPORT #%lu ======\r\n",
                   (unsigned long)reportNumber);
            printf("--- Analog (NTC) ---\r\n");
            printf("  Raw ADC:     %u\r\n", localSensor.analogRaw);
            char resStr[10];
            dtostrf(localSensor.analogResistance, 1, 0, resStr);
            printf("  Resistance:  %s ohm\r\n", resStr);
            printf("  Temperature: %s C\r\n", aTempStr);
            printf("  Valid:       %s\r\n",
                   localSensor.analogValid ? "YES" : "NO");
            printf("  Alert State: %s",
                   alertFullLabel(localAlert.analogAlertState));
            if (localAlert.analogAlertState == ALERT_DEBOUNCE_HIGH ||
                localAlert.analogAlertState == ALERT_DEBOUNCE_LOW) {
                printf(" (%u/%u)",
                       localAlert.analogDebounceCount, ALERT_DEBOUNCE_COUNT);
            }
            printf("\r\n");

            printf("--- Digital (DS18B20) ---\r\n");
            printf("  Temperature: %s C\r\n", dTempStr);
            printf("  Valid:       %s\r\n",
                   localSensor.digitalValid ? "YES" : "NO");
            printf("  Alert State: %s",
                   alertFullLabel(localAlert.digitalAlertState));
            if (localAlert.digitalAlertState == ALERT_DEBOUNCE_HIGH ||
                localAlert.digitalAlertState == ALERT_DEBOUNCE_LOW) {
                printf(" (%u/%u)",
                       localAlert.digitalDebounceCount, ALERT_DEBOUNCE_COUNT);
            }
            printf("\r\n");

            printf("--- Thresholds ---\r\n");
            char thAH[8], thAL[8], thDH[8], thDL[8];
            dtostrf(ANALOG_THRESHOLD_HIGH,  4, 1, thAH);
            dtostrf(ANALOG_THRESHOLD_LOW,   4, 1, thAL);
            dtostrf(DIGITAL_THRESHOLD_HIGH, 4, 1, thDH);
            dtostrf(DIGITAL_THRESHOLD_LOW,  4, 1, thDL);
            printf("  Analog:  HIGH=%sC  LOW=%sC\r\n", thAH, thAL);
            printf("  Digital: HIGH=%sC  LOW=%sC\r\n", thDH, thDL);

            printf("--- Statistics ---\r\n");
            printf("  Readings:        %lu\r\n",
                   (unsigned long)localSensor.readingCount);
            printf("  Conditioning:    %lu cycles\r\n",
                   (unsigned long)localAlert.conditioningCycles);
            printf("  Analog Alerts:   %lu\r\n",
                   (unsigned long)localAlert.analogAlertCount);
            printf("  Digital Alerts:  %lu\r\n",
                   (unsigned long)localAlert.digitalAlertCount);
            printf("================================\r\n");
        }
    }
}
