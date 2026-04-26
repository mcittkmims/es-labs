/**
 * @file task_display.cpp
 * @brief Lab 5.2 LCD and Serial Plotter reporting task.
 */

#include "task_display.h"
#include "lab5_2_config.h"
#include "shared_state.h"
#include "LcdDisplay.h"

#include <Arduino_FreeRTOS.h>
#include <math.h>
#include <stdio.h>
#include <string.h>

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

void vTaskLab5PidDisplay(void *pvParameters) {
    (void)pvParameters;

    s_lcd.init();
    s_lcd.backlight(true);
    s_lcd.showTwoLines("Lab 5.2 PID", "DHT11 + Fan");

    TickType_t lastWake = xTaskGetTickCount();
    const TickType_t period = pdMS_TO_TICKS(TASK_DISPLAY_PERIOD_MS);
    uint8_t displayCycle = 0;

    for (;;) {
        vTaskDelayUntil(&lastWake, period);
        displayCycle++;

        lab5PidStateLock();
        Lab5PidState snapshot = *lab5PidStateGet();
        lab5PidStateUnlock();

        char tempStr[8];
        char spStr[8];
        char outStr[8];
        char errStr[8];
        char kpStr[8];
        char kiStr[8];
        char kdStr[8];

        formatFloat(tempStr, sizeof(tempStr), snapshot.measuredTempC,
                    4, 1, "--.-");
        formatFloat(spStr, sizeof(spStr), snapshot.activeSetpointC,
                    4, 1, "--.-");
        formatFloat(outStr, sizeof(outStr), snapshot.appliedDutyPercent,
                    4, 0, "---");
        formatFloat(errStr, sizeof(errStr), snapshot.errorC,
                    4, 1, "-.-");
        formatFloat(kpStr, sizeof(kpStr), snapshot.kp,
                    4, 1, "-.-");
        formatFloat(kiStr, sizeof(kiStr), snapshot.ki,
                    4, 2, "-.--");
        formatFloat(kdStr, sizeof(kdStr), snapshot.kd,
                    4, 1, "-.-");

        char line0[17];
        char line1[17];

        if (snapshot.editingSetpoint) {
            snprintf(line0, sizeof(line0), "Set SP:%-3s C", snapshot.inputBuffer);
            snprintf(line1, sizeof(line1), "#=OK *=CLR");
        } else if (((displayCycle / 4) % 3) == 0) {
            snprintf(line0, sizeof(line0), "T:%s SP:%s", tempStr, spStr);
            snprintf(line1, sizeof(line1), "Fan:%s%% %s",
                     outStr,
                     snapshot.sensorValid ? "OK" : "SERR");
        } else if (((displayCycle / 4) % 3) == 1) {
            const char *source =
                snapshot.setpointSource == SETPOINT_SOURCE_POT ? "POT" : "MAN";
            snprintf(line0, sizeof(line0), "Err:%s Out:%s", errStr, outStr);
            snprintf(line1, sizeof(line1), "%s %s",
                     source,
                     PID_PRESETS[snapshot.pidPresetIndex].name);
        } else {
            snprintf(line0, sizeof(line0), "P:%s I:%s", kpStr, kiStr);
            snprintf(line1, sizeof(line1), "D:%s Duty:%s", kdStr, outStr);
        }

        s_lcd.showTwoLines(line0, line1);

        char plotSetpoint[10];
        char plotValue[10];
        char plotOutput[10];
        char plotDuty[10];
        char plotError[10];
        char plotKp[10];
        char plotKi[10];
        char plotKd[10];

        dtostrf(plotValueOrZero(snapshot.activeSetpointC), 1, 1, plotSetpoint);
        dtostrf(plotValueOrZero(snapshot.measuredTempC), 1, 1, plotValue);
        dtostrf(plotValueOrZero(snapshot.controlOutputPercent), 1, 1, plotOutput);
        dtostrf(plotValueOrZero(snapshot.appliedDutyPercent), 1, 1, plotDuty);
        dtostrf(plotValueOrZero(snapshot.errorC), 1, 1, plotError);
        dtostrf(plotValueOrZero(snapshot.kp), 1, 1, plotKp);
        dtostrf(plotValueOrZero(snapshot.ki), 1, 2, plotKi);
        dtostrf(plotValueOrZero(snapshot.kd), 1, 1, plotKd);

        printf("SetPoint:%s Value:%s Output:%s Duty:%s Error:%s Kp:%s Ki:%s Kd:%s Valid:%u\r\n",
               plotSetpoint,
               plotValue,
               plotOutput,
               plotDuty,
               plotError,
               plotKp,
               plotKi,
               plotKd,
               snapshot.sensorValid ? 1U : 0U);
    }
}
