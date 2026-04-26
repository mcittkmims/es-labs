/**
 * @file lab5_1_main.cpp
 * @brief Lab 5.1 - ON-OFF temperature control with hysteresis.
 */

#include "lab5_1_main.h"
#include "lab5_1_config.h"
#include "shared_state.h"
#include "task_input.h"
#include "task_acquisition.h"
#include "task_control.h"
#include "task_actuation.h"
#include "task_display.h"

#include <Arduino.h>
#include <Arduino_FreeRTOS.h>
#include <stdio.h>

#include "StdioSerial.h"

void lab5_1Setup() {
    stdioSerialInit(9600);

    printf("\r\n");
    printf("================================================\r\n");
    printf("  Lab 5.1 - ON-OFF Control with Hysteresis\r\n");
    printf("  Variant A: DHT22 temperature + relay actuator\r\n");
    printf("  Arduino Mega | FreeRTOS | LCD | Keypad\r\n");
    printf("================================================\r\n");
    printf("KEYPAD:\r\n");
    printf("  A = toggle setpoint source POT/MANUAL\r\n");
    printf("  B/C = decrease/increase manual setpoint by 0.5 C\r\n");
    printf("  D = cycle hysteresis band 0.5..5.0 C\r\n");
    printf("  digits + # = enter integer manual setpoint\r\n");
    printf("  * = cancel numeric entry\r\n");
    printf("PINS:\r\n");
    printf("  DHT22 data: D%u\r\n", (unsigned)PIN_DHT22);
    printf("  Relay IN:   D%u\r\n", (unsigned)PIN_RELAY);
    printf("  Pot SIG:    A0\r\n");
    printf("  LCD:        SDA/SCL\r\n");
    printf("PLOTTER LINE:\r\n");
    printf("  SetPoint:<C> Value:<C> Output:<0/1> Low:<C> High:<C>\r\n");
    printf("================================================\r\n\r\n");

    lab5StateInit();

    BaseType_t okInput = xTaskCreate(
        vTaskLab5Input,
        "Input",
        TASK_INPUT_STACK,
        NULL,
        TASK_INPUT_PRIORITY,
        NULL
    );

    BaseType_t okAcquisition = xTaskCreate(
        vTaskLab5Acquisition,
        "Acquire",
        TASK_ACQUISITION_STACK,
        NULL,
        TASK_ACQUISITION_PRIORITY,
        NULL
    );

    BaseType_t okControl = xTaskCreate(
        vTaskLab5Control,
        "Control",
        TASK_CONTROL_STACK,
        NULL,
        TASK_CONTROL_PRIORITY,
        NULL
    );

    BaseType_t okActuation = xTaskCreate(
        vTaskLab5Actuation,
        "Actuate",
        TASK_ACTUATION_STACK,
        NULL,
        TASK_ACTUATION_PRIORITY,
        NULL
    );

    BaseType_t okDisplay = xTaskCreate(
        vTaskLab5Display,
        "Display",
        TASK_DISPLAY_STACK,
        NULL,
        TASK_DISPLAY_PRIORITY,
        NULL
    );

    if (okInput != pdPASS || okAcquisition != pdPASS ||
        okControl != pdPASS || okActuation != pdPASS ||
        okDisplay != pdPASS) {
        printf("[ERROR] Task creation failed: I=%ld A=%ld C=%ld R=%ld D=%ld\r\n",
               (long)okInput,
               (long)okAcquisition,
               (long)okControl,
               (long)okActuation,
               (long)okDisplay);
    }
}

void lab5_1Loop() {
    // All application logic runs in FreeRTOS tasks.
}
