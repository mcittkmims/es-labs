/**
 * @file lab3_1_main.cpp
 * @brief Lab 3.1 — Dual-Sensor Temperature Monitoring with FreeRTOS
 *
 * Entry point for Laboratory Work 3.1. This file initializes all hardware
 * peripherals, creates FreeRTOS synchronization primitives, and spawns
 * three preemptive tasks that together implement a dual-sensor temperature
 * monitoring system with hysteresis-based threshold alerting.
 *
 * ──────────────────────────────────────────────────────────────────────────
 * System overview
 * ──────────────────────────────────────────────────────────────────────────
 *
 * This application monitors two temperature sensors simultaneously:
 *   - Analog: NTC thermistor (10K, Beta=3950) via ADC on pin A0
 *   - Digital: DS18B20 via OneWire protocol on pin D2
 *
 * Temperature readings are processed through a signal conditioning
 * pipeline that applies hysteresis-based threshold detection with
 * debounce filtering to prevent false alerts near threshold boundaries.
 *
 * ┌──────────────┬────────────┬──────┬──────────────────────────────────────┐
 * │ Task         │ Trigger    │ Prio │ Responsibility                       │
 * ├──────────────┼────────────┼──────┼──────────────────────────────────────┤
 * │ Acquisition  │ 50 ms      │  3   │ Read NTC (ADC) + DS18B20 (OneWire)  │
 * │              │ DelayUntil │      │ Store in shared data, give semaphore │
 * ├──────────────┼────────────┼──────┼──────────────────────────────────────┤
 * │ Conditioning │ Semaphore  │  2   │ Hysteresis threshold + debounce     │
 * │              │ event      │      │ Alert FSM, LED control              │
 * ├──────────────┼────────────┼──────┼──────────────────────────────────────┤
 * │ Display      │ 500 ms     │  1   │ LCD update + STDIO structured report│
 * │              │ DelayUntil │      │ Temperatures, states, statistics    │
 * └──────────────┴────────────┴──────┴──────────────────────────────────────┘
 *
 * ──────────────────────────────────────────────────────────────────────────
 * Synchronization mechanisms
 * ──────────────────────────────────────────────────────────────────────────
 *
 *   Binary Semaphore (xNewReadingSemaphore):
 *     Given by Task 1 after each acquisition cycle.
 *     Taken by Task 2 to process the new readings.
 *
 *   Mutex (xSensorMutex):
 *     Protects g_sensorData (Task 1 writes, Task 2/3 read) and
 *     g_alertData (Task 2 writes, Task 3 reads). Provides priority
 *     inheritance to prevent priority inversion.
 */

#include "lab3_1_main.h"
#include "sensor_data.h"
#include "task_acquisition.h"
#include "task_conditioning.h"
#include "task_display.h"

#include <Arduino.h>
#include <Arduino_FreeRTOS.h>
#include <stdio.h>

#include "StdioSerial.h"
#include <stdlib.h>  // for dtostrf on AVR

// ──────────────────────────────────────────────────────────────────────────
// Lab 3.1 public entry points
// ──────────────────────────────────────────────────────────────────────────

void lab3_1Setup() {
    // ── Initialize STDIO serial at 9600 baud ──────────────────────────
    stdioSerialInit(9600);

    // ── Print startup banner ───────────────────────────────────────────
    printf("\r\n");
    printf("================================================\r\n");
    printf("  Lab 3.1 — Dual-Sensor Temperature Monitor\r\n");
    printf("  FreeRTOS Preemptive Scheduler\r\n");
    printf("  Tasks: 3 | Tick: %u Hz\r\n",
           (unsigned int)configTICK_RATE_HZ);
    printf("================================================\r\n");
    printf("SENSORS:\r\n");
    printf("  Analog:  NTC Thermistor (A0, 10K, Beta=3950)\r\n");
    printf("  Digital: DS18B20 (Pin 2, OneWire, 10-bit)\r\n");
    printf("THRESHOLDS:\r\n");
    // AVR printf does NOT support %f — use dtostrf + %s.
    char ahBuf[8], alBuf[8], dhBuf[8], dlBuf[8];
    dtostrf(ANALOG_THRESHOLD_HIGH,  4, 1, ahBuf);
    dtostrf(ANALOG_THRESHOLD_LOW,   4, 1, alBuf);
    dtostrf(DIGITAL_THRESHOLD_HIGH, 4, 1, dhBuf);
    dtostrf(DIGITAL_THRESHOLD_LOW,  4, 1, dlBuf);
    printf("  Analog:  HIGH=%sC  LOW=%sC\r\n", ahBuf, alBuf);
    printf("  Digital: HIGH=%sC  LOW=%sC\r\n", dhBuf, dlBuf);
    printf("  Debounce: %u confirmations\r\n",
           (unsigned int)ALERT_DEBOUNCE_COUNT);
    printf("LEDs:\r\n");
    printf("  GREEN  = system normal (no alerts)\r\n");
    printf("  RED    = analog sensor alert\r\n");
    printf("  YELLOW = digital sensor alert\r\n");
    printf("TIMING:\r\n");
    printf("  Acquisition:   %u ms\r\n",
           (unsigned int)TASK_ACQUISITION_PERIOD_MS);
    printf("  Display/Report: %u ms\r\n",
           (unsigned int)TASK_DISPLAY_PERIOD_MS);
    printf("  STDIO Report:  every 2 seconds\r\n");
    printf("================================================\r\n\r\n");

    // ── Create synchronization primitives ──────────────────────────────
    sensorDataInit();

    // ── Create FreeRTOS tasks ──────────────────────────────────────────
    xTaskCreate(
        vTaskAcquisition,
        "Acquire",
        TASK_ACQUISITION_STACK,
        NULL,
        TASK_ACQUISITION_PRIORITY,
        NULL
    );

    xTaskCreate(
        vTaskConditioning,
        "Cond",
        TASK_CONDITIONING_STACK,
        NULL,
        TASK_CONDITIONING_PRIORITY,
        NULL
    );

    xTaskCreate(
        vTaskDisplay,
        "Display",
        TASK_DISPLAY_STACK,
        NULL,
        TASK_DISPLAY_PRIORITY,
        NULL
    );

    // The FreeRTOS scheduler starts automatically after setup() returns
    // (handled by the Arduino_FreeRTOS library integration).
}

void lab3_1Loop() {
    // All application logic runs inside the three FreeRTOS tasks.
    // This function intentionally does nothing.
}
