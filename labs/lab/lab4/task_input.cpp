/**
 * @file task_input.cpp
 * @brief Lab 4 — Keypad Input Task Implementation
 *
 * Scans the keypad and updates the shared state based on user commands.
 * Command mapping:
 *   A = Toggle relay (binary actuator ON/OFF)
 *   B = Enter analog input mode (type PWM %)
 *   0-9 = Digit entry for analog value
 *   # = Confirm analog value entry
 *   * = Cancel current input / clear
 *   C = Emergency stop (both actuators off)
 *   D = Request serial status report
 */

#include "task_input.h"
#include "shared_state.h"
#include "lab4_config.h"

#include "KeypadInput.h"
#include <stdio.h>
#include <stdlib.h>

static KeypadInput keypad(KEYPAD_ROW_PINS, KEYPAD_COL_PINS);

void vTaskInput(void *pvParameters) {
    (void)pvParameters;
    keypad.init();

    TickType_t xLastWake = xTaskGetTickCount();
    const TickType_t xPeriod = pdMS_TO_TICKS(TASK_INPUT_PERIOD_MS);

    for (;;) {
        char key = keypad.getKey();

        if (key != 0) {
            sharedStateLock();
            ActuatorState *s = sharedStateGet();

            switch (key) {
                case 'A':
                    // Toggle relay command
                    s->relayCommandOn = !s->relayCommandOn;
                    s->inputModeAnalog = false;
                    printf("[INPUT] Relay cmd: %s\r\n",
                           s->relayCommandOn ? "ON" : "OFF");
                    break;

                case 'B':
                    // Switch to analog input mode
                    s->inputModeAnalog = true;
                    s->inputBufferLen = 0;
                    s->inputBuffer[0] = '\0';
                    printf("[INPUT] Analog mode — enter 0-100 then #\r\n");
                    break;

                case 'C':
                    // Emergency stop
                    s->relayCommandOn = false;
                    s->pwmCommandPercent = 0.0f;
                    s->inputModeAnalog = false;
                    s->inputBufferLen = 0;
                    printf("[INPUT] EMERGENCY STOP\r\n");
                    break;

                case 'D':
                    // Status report request (handled by display task)
                    printf("[INPUT] Status report requested\r\n");
                    break;

                case '#':
                    // Confirm analog value
                    if (s->inputModeAnalog && s->inputBufferLen > 0) {
                        s->inputBuffer[s->inputBufferLen] = '\0';
                        int val = atoi(s->inputBuffer);
                        if (val < 0) val = 0;
                        if (val > 100) val = 100;
                        s->pwmCommandPercent = (float)val;
                        printf("[INPUT] PWM set to %d%%\r\n", val);
                        s->inputBufferLen = 0;
                        s->inputModeAnalog = false;
                    }
                    break;

                case '*':
                    // Cancel input
                    s->inputBufferLen = 0;
                    s->inputModeAnalog = false;
                    printf("[INPUT] Input cancelled\r\n");
                    break;

                default:
                    // Digit entry (0-9)
                    if (key >= '0' && key <= '9' && s->inputModeAnalog) {
                        if (s->inputBufferLen < 3) {
                            s->inputBuffer[s->inputBufferLen++] = key;
                            s->inputBuffer[s->inputBufferLen] = '\0';
                            printf("[INPUT] Entering: %s%%\r\n", s->inputBuffer);
                        }
                    }
                    break;
            }

            sharedStateUnlock();
        }

        vTaskDelayUntil(&xLastWake, xPeriod);
    }
}
