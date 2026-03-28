/**
 * @file task_input.cpp
 * @brief Input Reader Task Implementation — Lab 4.1
 *
 * Polls the hardware serial port at TASK_INPUT_PERIOD_MS (10 ms) intervals.
 * Incoming characters are accumulated into a line buffer with local echo.
 * When a newline is received the buffer is trimmed, uppercased, and matched
 * against "ON" / "OFF". Valid commands are written to g_actuator.pendingCmd
 * under mutex protection; unrecognized input triggers an error message.
 *
 * Backspace (0x08 or 0x7F) removes the last character from the buffer.
 * The prompt "> " is reprinted after each completed command.
 */

#include "task_input.h"
#include "actuator_data.h"
#include <Arduino.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>

// ── Internal helpers ──────────────────────────────────────────────────────────

/**
 * @brief Trim leading/trailing whitespace and convert to upper-case in-place.
 */
static void normalizeInput(char *str) {
    // Trim leading whitespace
    char *start = str;
    while (*start && isspace((unsigned char)*start)) { start++; }

    // Trim trailing whitespace
    int len = (int)strlen(start);
    while (len > 0 && isspace((unsigned char)start[len - 1])) { len--; }
    start[len] = '\0';

    // Shift left if leading spaces were removed, then uppercase
    if (start != str) { memmove(str, start, (size_t)(len + 1)); }
    for (char *p = str; *p; p++) { *p = (char)toupper((unsigned char)*p); }
}

// ── Task implementation ───────────────────────────────────────────────────────

void vTaskInputReader(void *params) {
    (void)params;

    char    buf[32];
    uint8_t len = 0;

    printf("Commands: ON | OFF\r\n> ");

    TickType_t lastWake = xTaskGetTickCount();

    while (true) {
        // Drain all available serial bytes in this polling window
        while (Serial.available()) {
            char c = (char)Serial.read();
            Serial.write(c);  // local echo

            if (c == '\r' || c == '\n') {
                Serial.write('\n');
                if (len > 0) {
                    buf[len] = '\0';
                    normalizeInput(buf);

                    // Match normalized input against known commands
                    ActuatorControl::State cmd;
                    bool valid = true;

                    if (strcmp(buf, "ON") == 0) {
                        cmd = ActuatorControl::State::ON;
                    } else if (strcmp(buf, "OFF") == 0) {
                        cmd = ActuatorControl::State::OFF;
                    } else {
                        printf("  [ERR] Unknown command: '%s'. Use ON or OFF.\r\n", buf);
                        valid = false;
                    }

                    if (valid) {
                        if (xSemaphoreTake(g_actuator.mutex, pdMS_TO_TICKS(5)) == pdTRUE) {
                            g_actuator.pendingCmd = cmd;
                            xSemaphoreGive(g_actuator.mutex);
                        }
                        printf("  >> Accepted: %s\r\n",
                               (cmd == ActuatorControl::State::ON) ? "ON" : "OFF");
                    }

                    len = 0;
                    printf("> ");
                }
            } else if (c == '\b' || c == 127) {
                if (len > 0) {
                    len--;
                    Serial.print("\b \b");
                }
            } else if (len < (sizeof(buf) - 1)) {
                buf[len++] = c;
            }
        }

        vTaskDelayUntil(&lastWake, pdMS_TO_TICKS(TASK_INPUT_PERIOD_MS));
    }
}
