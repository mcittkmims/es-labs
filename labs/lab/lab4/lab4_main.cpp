/**
 * @file lab4_main.cpp
 * @brief Lab 4 — Dual Actuator Control System Entry Point
 *
 * Initializes STDIO serial, shared state, and spawns three FreeRTOS
 * tasks for keypad input, actuator control with conditioning, and
 * LCD/serial display reporting.
 */

#include "lab4_main.h"
#include "lab4_config.h"
#include "shared_state.h"
#include "task_input.h"
#include "task_control.h"
#include "task_display.h"

#include <Arduino.h>
#include <Arduino_FreeRTOS.h>
#include <stdio.h>

#include "StdioSerial.h"

void lab4Setup() {
    // Initialize STDIO serial
    stdioSerialInit(9600);

    // Print startup banner
    printf("\r\n");
    printf("================================================\r\n");
    printf("  Lab 4 — Dual Actuator Control System\r\n");
    printf("  Binary (Relay) + Analog (PWM)\r\n");
    printf("  Keypad Interface | FreeRTOS\r\n");
    printf("  Varianta C (100%%)\r\n");
    printf("================================================\r\n");
    printf("COMMANDS (Keypad):\r\n");
    printf("  A = Toggle relay ON/OFF\r\n");
    printf("  B = Enter PWM value mode\r\n");
    printf("  0-9 = Digit entry (PWM %%)\r\n");
    printf("  # = Confirm PWM value\r\n");
    printf("  * = Cancel input\r\n");
    printf("  C = Emergency stop (all OFF)\r\n");
    printf("  D = Print status report\r\n");
    printf("HARDWARE:\r\n");
    printf("  Relay:    pin D%d\r\n", PIN_RELAY);
    printf("  PWM out:  pin D%d\r\n", PIN_PWM_ACT);
    printf("  LED grn:  pin D%d\r\n", PIN_LED_GREEN);
    printf("  LED red:  pin D%d\r\n", PIN_LED_RED);
    printf("CONDITIONING:\r\n");
    printf("  Median=%u, EWMA=0.4, Ramp=5%%/cycle\r\n",
           (unsigned)ACT_MEDIAN_WINDOW);
    printf("  Overload: >90%% alert, <80%% clear\r\n");
    printf("================================================\r\n\r\n");

    // Initialize shared state + mutex
    sharedStateInit();

    // Create FreeRTOS tasks (report creation failures explicitly)
    BaseType_t okInput = xTaskCreate(vTaskInput, "Input", TASK_INPUT_STACK,
                                     NULL, TASK_INPUT_PRIORITY, NULL);
    BaseType_t okControl = xTaskCreate(vTaskControl, "Control", TASK_CONTROL_STACK,
                                       NULL, TASK_CONTROL_PRIORITY, NULL);
    BaseType_t okDisplay = xTaskCreate(vTaskDisplay, "Display", TASK_DISPLAY_STACK,
                                       NULL, TASK_DISPLAY_PRIORITY, NULL);

    if (okInput != pdPASS || okControl != pdPASS || okDisplay != pdPASS) {
        printf("[ERROR] Task creation failed: Input=%ld Control=%ld Display=%ld\r\n",
               (long)okInput, (long)okControl, (long)okDisplay);
    }
}

void lab4Loop() {
    // All logic runs in FreeRTOS tasks
}
