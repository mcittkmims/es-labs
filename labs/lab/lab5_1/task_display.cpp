/**
 * @file task_display.cpp
 * @brief Lab 5.1 LCD and Serial Plotter reporting task.
 */

#include "task_display.h"
#include "lab5_1_config.h"
#include "shared_state.h"
#include "LcdDisplay.h"

#include <Arduino_FreeRTOS.h>
#include <stdio.h>
#include <string.h>
#include <math.h>

static LcdDisplay s_lcd(LCD_I2C_ADDRESS, LCD_COLS, LCD_ROWS);

static void formatFloat(char *buffer, size_t size, float value,
                        signed char width, unsigned char precision,
                        const char *invalidText) {
    if (isnan(value)) {
        strncpy(buffer, invalidText, size);
        buffer[size - 1] = '\0';
    } else {
        dtostrf(value, width, precision, buffer);
    }
}

static float plotValueOrZero(float value) {
    return isnan(value) ? 0.0f : value;
}

void vTaskLab5Display(void *pvParameters) {
    (void)pvParameters;

    s_lcd.init();
    s_lcd.backlight(true);
    s_lcd.showTwoLines("Lab 5.1 ONOFF", "DHT11 + Relay");

    TickType_t lastWake = xTaskGetTickCount();
    const TickType_t period = pdMS_TO_TICKS(TASK_DISPLAY_PERIOD_MS);
    uint8_t displayCycle = 0;

    for (;;) {
        vTaskDelayUntil(&lastWake, period);
        displayCycle++;

        lab5StateLock();
        Lab5ControlState snapshot = *lab5StateGet();
        lab5StateUnlock();

        char tempStr[8];
        char humStr[8];
        char spStr[8];
        char hystStr[8];
        char lowStr[8];
        char highStr[8];

        formatFloat(tempStr, sizeof(tempStr), snapshot.measuredTempC,
                    4, 1, "--.-");
        formatFloat(humStr, sizeof(humStr), snapshot.measuredHumidityPercent,
                    4, 0, "---");
        formatFloat(spStr, sizeof(spStr), snapshot.activeSetpointC,
                    4, 1, "--.-");
        formatFloat(hystStr, sizeof(hystStr), snapshot.hysteresisBandC,
                    3, 1, "-.-");
        formatFloat(lowStr, sizeof(lowStr), snapshot.lowerThresholdC,
                    4, 1, "--.-");
        formatFloat(highStr, sizeof(highStr), snapshot.upperThresholdC,
                    4, 1, "--.-");

        char line0[17];
        char line1[17];

        if (snapshot.editingSetpoint) {
            snprintf(line0, sizeof(line0), "Set SP:%-3s C", snapshot.inputBuffer);
            snprintf(line1, sizeof(line1), "#=OK *=CLR");
        } else if (((displayCycle / 4) % 2) == 0) {
            snprintf(line0, sizeof(line0), "T:%s SP:%s", tempStr, spStr);
            snprintf(line1, sizeof(line1), "Relay:%-3s %s",
                     snapshot.actuatorOn ? "ON" : "OFF",
                     snapshot.sensorValid ? "OK" : "SERR");
        } else {
            const char *source =
                snapshot.setpointSource == SETPOINT_SOURCE_POT ? "POT" : "MAN";
            snprintf(line0, sizeof(line0), "H:%s Hum:%s%%", hystStr, humStr);
            snprintf(line1, sizeof(line1), "%s %s-%s", source, lowStr, highStr);
        }

        s_lcd.showTwoLines(line0, line1);

        char plotSetpoint[10];
        char plotValue[10];
        char plotLow[10];
        char plotHigh[10];
        dtostrf(plotValueOrZero(snapshot.activeSetpointC), 1, 1, plotSetpoint);
        dtostrf(plotValueOrZero(snapshot.measuredTempC), 1, 1, plotValue);
        dtostrf(plotValueOrZero(snapshot.lowerThresholdC), 1, 1, plotLow);
        dtostrf(plotValueOrZero(snapshot.upperThresholdC), 1, 1, plotHigh);

        printf("SetPoint:%s Value:%s Output:%u Low:%s High:%s Valid:%u\r\n",
               plotSetpoint,
               plotValue,
               snapshot.actuatorOn ? 1U : 0U,
               plotLow,
               plotHigh,
               snapshot.sensorValid ? 1U : 0U);
    }
}
