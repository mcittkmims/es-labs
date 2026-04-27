/**
 * @file lab5_2_main.cpp
 * @brief Lab 5.2 - PID temperature control with PWM fan actuator.
 */

#include "lab5_2_main.h"
#include "lab5_2_config.h"
#include "shared_state.h"
#include "task_input.h"
#include "task_acquisition.h"
#include "task_control.h"
#include "task_actuation.h"
#include "task_display.h"

#include <Arduino.h>
#include <Arduino_FreeRTOS.h>
#include <stdio.h>

extern "C" void vApplicationStackOverflowHook(TaskHandle_t xTask,
                                                 char *pcTaskName) {
    (void)xTask;
    printf("[FATAL] Stack overflow in task: %s\r\n",
           pcTaskName != NULL ? pcTaskName : "<unknown>");
    taskDISABLE_INTERRUPTS();
    for (;;) {}
}

extern "C" void vApplicationMallocFailedHook(void) {
    printf("[FATAL] FreeRTOS malloc failed\r\n");
    taskDISABLE_INTERRUPTS();
    for (;;) {}
}

#include "StdioSerial.h"

void lab5_2Setup() {
    stdioSerialInit(9600);

    printf("\r\n");
    printf("================================================\r\n");
    printf("  Lab 5.2 - PID Control\r\n");
    printf("  Variant: DHT11 temperature + PWM fan via L293D\r\n");
    printf("  Arduino Mega | FreeRTOS | LCD | Keypad\r\n");
    printf("================================================\r\n");
    printf("KEYPAD:\r\n");
    printf("  A = toggle setpoint source POT/MANUAL\r\n");
    printf("  B/C = decrease/increase manual setpoint by 0.5 C\r\n");
    printf("  D = cycle PID preset SOFT/NORM/FAST\r\n");
    printf("  digits + # = enter integer manual setpoint\r\n");
    printf("  * = cancel numeric entry\r\n");
    printf("PINS:\r\n");
    printf("  DHT11 data: D%u\r\n", (unsigned)PIN_DHT_SENSOR);
    printf("  Fan EN/PWM: D%u\r\n", (unsigned)PIN_FAN_PWM);
    printf("  Fan IN1:    D%u\r\n", (unsigned)PIN_FAN_IN1);
    printf("  Fan IN2:    D%u\r\n", (unsigned)PIN_FAN_IN2);
    printf("  Pot SIG:    A0\r\n");
    printf("  LCD:        SDA/SCL\r\n");
    printf("PLOTTER LINE:\r\n");
    printf("  SetPoint:<C> Value:<C> Output:<%%> Duty:<%%> Error:<C> Kp Ki Kd Valid\r\n");
    printf("================================================\r\n\r\n");

    lab5PidStateInit();

    BaseType_t okInput = xTaskCreate(
        vTaskLab5PidInput,
        "Input",
        TASK_INPUT_STACK,
        NULL,
        TASK_INPUT_PRIORITY,
        NULL
    );

    BaseType_t okAcquisition = xTaskCreate(
        vTaskLab5PidAcquisition,
        "Acquire",
        TASK_ACQUISITION_STACK,
        NULL,
        TASK_ACQUISITION_PRIORITY,
        NULL
    );

    BaseType_t okControl = xTaskCreate(
        vTaskLab5PidControl,
        "Control",
        TASK_CONTROL_STACK,
        NULL,
        TASK_CONTROL_PRIORITY,
        NULL
    );

    BaseType_t okActuation = xTaskCreate(
        vTaskLab5PidActuation,
        "Actuate",
        TASK_ACTUATION_STACK,
        NULL,
        TASK_ACTUATION_PRIORITY,
        NULL
    );

    BaseType_t okDisplay = xTaskCreate(
        vTaskLab5PidDisplay,
        "Display",
        TASK_DISPLAY_STACK,
        NULL,
        TASK_DISPLAY_PRIORITY,
        NULL
    );

    if (okInput != pdPASS || okAcquisition != pdPASS ||
        okControl != pdPASS || okActuation != pdPASS ||
        okDisplay != pdPASS) {
        printf("[ERROR] Task creation failed: I=%ld A=%ld C=%ld M=%ld D=%ld\r\n",
               (long)okInput,
               (long)okAcquisition,
               (long)okControl,
               (long)okActuation,
               (long)okDisplay);
    }
}

void lab5_2Loop() {
    // All application logic runs in FreeRTOS tasks.
}
