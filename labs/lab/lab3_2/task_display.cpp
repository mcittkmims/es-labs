/**
 * @file task_display.cpp
 * @brief Lab 3.2 — Display & Reporting Task Implementation (Task 3)
 *
 * Implements the periodic display and reporting task. Every 500 ms,
 * this task reads sensor data and alert status, updates the LCD with
 * a summary, and prints a structured report to the serial terminal
 * via STDIO printf that exposes all conditioning pipeline stages.
 *
 * ──────────────────────────────────────────────────────────────────────────
 * Display strategy
 * ──────────────────────────────────────────────────────────────────────────
 *
 * The 16x2 LCD alternates between two information pages every 2 seconds:
 *
 *   Page 0 — Current readings (conditioned EWMA values):
 *     Line 0: "A:25.3C  D:24.8C"
 *     Line 1: "System: OK" or "ALERT: Analog" / "ALERT: Digital" / "ALERT: A+D"
 *
 *   Page 1 — Conditioning and threshold configuration:
 *     Line 0: "Med:5 Alpha:0.3"
 *     Line 1: "TH:30 TL:28 C"
 *
 * ──────────────────────────────────────────────────────────────────────────
 * STDIO report format (every 2 seconds = every 4th display cycle)
 * ──────────────────────────────────────────────────────────────────────────
 *
 * Shows raw, median-filtered, and EWMA values for both sensors,
 * conditioning configuration, threshold settings, and statistics.
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
// Helper: get full alert state label for STDIO report
// ──────────────────────────────────────────────────────────────────────────

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
    s_lcd.showTwoLines("Lab 3.2 CondPipe", "Initializing...");

    // Allow sensors and conditioning pipeline to stabilize.
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

        // ── Read shared data under mutex ────────────────────────────────
        if (xSemaphoreTake(xSensorMutex, pdMS_TO_TICKS(20)) == pdTRUE) {
            memcpy(&localSensor, &g_sensorData, sizeof(SensorReadings_t));
            memcpy(&localAlert,  &g_alertData,  sizeof(AlertStatus_t));
            xSemaphoreGive(xSensorMutex);
        } else {
            continue;  // Skip this cycle if mutex unavailable.
        }

        // ── Update LCD display ──────────────────────────────────────────
        uint8_t page = (displayCycle / 4) % 2;

        if (page == 0) {
            // Page 0: Current conditioned readings and alert status.
            char aTemp[8], dTemp[8];
            formatTemp(aTemp, sizeof(aTemp), localSensor.analogEwma);
            formatTemp(dTemp, sizeof(dTemp), localSensor.digitalEwma);

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
            // Page 1: Conditioning config and thresholds.
            char alphaStr[6];
            dtostrf(EWMA_ALPHA, 3, 1, alphaStr);
            snprintf(line0, sizeof(line0), "Med:%u Alpha:%s",
                     (unsigned int)MEDIAN_WINDOW_SIZE, alphaStr);

            char thH[6], thL[6];
            dtostrf(ANALOG_THRESHOLD_HIGH, 3, 0, thH);
            dtostrf(ANALOG_THRESHOLD_LOW,  3, 0, thL);
            snprintf(line1, sizeof(line1), "TH:%s TL:%s C", thH, thL);
        }

        s_lcd.showTwoLines(line0, line1);

        // ── Structured STDIO report (every 2 seconds) ──────────────────
        if ((displayCycle % REPORT_INTERVAL) == 0) {
            reportNumber++;

            // Format temperature strings (AVR printf does not support %f).
            char aRawStr[8], aMedianStr[8], aEwmaStr[8];
            char dRawStr[8], dMedianStr[8], dEwmaStr[8];
            formatTemp(aRawStr,    sizeof(aRawStr),    localSensor.analogTempRaw);
            formatTemp(aMedianStr, sizeof(aMedianStr), localSensor.analogMedian);
            formatTemp(aEwmaStr,   sizeof(aEwmaStr),   localSensor.analogEwma);
            formatTemp(dRawStr,    sizeof(dRawStr),    localSensor.digitalTempRaw);
            formatTemp(dMedianStr, sizeof(dMedianStr), localSensor.digitalMedian);
            formatTemp(dEwmaStr,   sizeof(dEwmaStr),   localSensor.digitalEwma);

            printf("\r\n");
            printf("====== SENSOR REPORT #%lu ======\r\n",
                   (unsigned long)reportNumber);

            // ── Analog sensor section ───────────────────────────────────
            printf("--- Analog (NTC) ---\r\n");
            printf("  Raw ADC:     %u\r\n", localSensor.analogRaw);
            char resStr[10];
            dtostrf(localSensor.analogResistance, 1, 0, resStr);
            printf("  Resistance:  %s ohm\r\n", resStr);
            printf("  Temperature: %s C (raw)\r\n", aRawStr);
            printf("  After Median: %s C\r\n", aMedianStr);
            printf("  After EWMA:  %s C (final)\r\n", aEwmaStr);
            printf("  Valid:       %s\r\n",
                   localSensor.analogValid ? "YES" : "NO");
            printf("  Conditioned: %s\r\n",
                   localSensor.analogConditioned ? "YES" : "FILLING");
            printf("  Alert State: %s",
                   alertFullLabel(localAlert.analogAlertState));
            if (localAlert.analogAlertState == ALERT_DEBOUNCE_HIGH ||
                localAlert.analogAlertState == ALERT_DEBOUNCE_LOW) {
                printf(" (%u/%u)",
                       localAlert.analogDebounceCount, ALERT_DEBOUNCE_COUNT);
            }
            printf("\r\n");

            // ── Digital sensor section ──────────────────────────────────
            printf("--- Digital (DS18B20) ---\r\n");
            printf("  Temperature: %s C (raw)\r\n", dRawStr);
            printf("  After Median: %s C\r\n", dMedianStr);
            printf("  After EWMA:  %s C (final)\r\n", dEwmaStr);
            printf("  Valid:       %s\r\n",
                   localSensor.digitalValid ? "YES" : "NO");
            printf("  Conditioned: %s\r\n",
                   localSensor.digitalConditioned ? "YES" : "FILLING");
            printf("  Alert State: %s",
                   alertFullLabel(localAlert.digitalAlertState));
            if (localAlert.digitalAlertState == ALERT_DEBOUNCE_HIGH ||
                localAlert.digitalAlertState == ALERT_DEBOUNCE_LOW) {
                printf(" (%u/%u)",
                       localAlert.digitalDebounceCount, ALERT_DEBOUNCE_COUNT);
            }
            printf("\r\n");

            // ── Conditioning configuration ──────────────────────────────
            printf("--- Conditioning Config ---\r\n");
            printf("  Median Window: %u samples\r\n",
                   (unsigned int)MEDIAN_WINDOW_SIZE);
            char alphaStr[6];
            dtostrf(EWMA_ALPHA, 3, 1, alphaStr);
            printf("  EWMA Alpha:   %s\r\n", alphaStr);
            char satMinStr[8], satMaxStr[8];
            dtostrf(SATURATION_MIN, 4, 1, satMinStr);
            dtostrf(SATURATION_MAX, 5, 1, satMaxStr);
            printf("  Saturation:   [%s, %s] C\r\n", satMinStr, satMaxStr);

            // ── Thresholds ──────────────────────────────────────────────
            printf("--- Thresholds ---\r\n");
            char thAH[8], thAL[8];
            dtostrf(ANALOG_THRESHOLD_HIGH, 4, 1, thAH);
            dtostrf(ANALOG_THRESHOLD_LOW,  4, 1, thAL);
            printf("  HIGH: %s C   LOW: %s C\r\n", thAH, thAL);
            printf("  Debounce: %u confirmations\r\n",
                   (unsigned int)ALERT_DEBOUNCE_COUNT);

            // ── Statistics ──────────────────────────────────────────────
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
