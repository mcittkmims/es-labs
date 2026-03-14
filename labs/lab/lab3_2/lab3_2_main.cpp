/**
 * @file lab3_2_main.cpp
 * @brief Lab 3.2 — Dual-Sensor Temperature Monitoring with Signal Conditioning
 *
 * Entry point for Laboratory Work 3.2. This file initializes all hardware
 * peripherals, creates FreeRTOS synchronization primitives, and spawns
 * three preemptive tasks that together implement a dual-sensor temperature
 * monitoring system with a three-stage signal conditioning pipeline.
 *
 * ──────────────────────────────────────────────────────────────────────────
 * System overview
 * ──────────────────────────────────────────────────────────────────────────
 *
 * This application monitors two temperature sensors simultaneously:
 *   - Analog: NTC thermistor (10K, Beta=3950) via ADC on pin A0
 *   - Digital: DS18B20 via OneWire protocol on pin D2
 *
 * Raw sensor readings are processed through a signal conditioning pipeline:
 *   1. Saturation (clamping to valid range)
 *   2. Median filter (salt-and-pepper noise removal)
 *   3. EWMA (exponentially weighted moving average for smoothing)
 *
 * The conditioned values are then evaluated by a hysteresis-based
 * threshold alert FSM with debounce filtering.
 *
 * ┌──────────────┬────────────┬──────┬──────────────────────────────────────┐
 * │ Task         │ Trigger    │ Prio │ Responsibility                       │
 * ├──────────────┼────────────┼──────┼──────────────────────────────────────┤
 * │ Acquisition  │ 50 ms      │  3   │ Read NTC (ADC) + DS18B20 (OneWire)  │
 * │              │ DelayUntil │      │ Store raw data, give semaphore       │
 * ├──────────────┼────────────┼──────┼──────────────────────────────────────┤
 * │ Conditioning │ Semaphore  │  2   │ Saturate → Median → EWMA pipeline   │
 * │              │ event      │      │ Threshold alert FSM, LED control     │
 * ├──────────────┼────────────┼──────┼──────────────────────────────────────┤
 * │ Display      │ 500 ms     │  1   │ LCD update + STDIO structured report │
 * │              │ DelayUntil │      │ Raw + conditioned values, statistics │
 * └──────────────┴────────────┴──────┴──────────────────────────────────────┘
 */

#include "lab3_2_main.h"
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
// Lab 3.2 public entry points
// ──────────────────────────────────────────────────────────────────────────

void lab3_2Setup() {
    // ── Initialize STDIO serial at 9600 baud ────────────────────────────
    stdioSerialInit(9600);

    // ── Print startup banner ────────────────────────────────────────────
    printf("\r\n");
    printf("================================================\r\n");
    printf("  Lab 3.2 — Signal Conditioning Pipeline\r\n");
    printf("  Dual-Sensor Temperature Monitor\r\n");
    printf("  FreeRTOS Preemptive Scheduler\r\n");
    printf("  Tasks: 3 | Tick: %u Hz\r\n",
           (unsigned int)configTICK_RATE_HZ);
    printf("================================================\r\n");
    printf("SENSORS:\r\n");
    printf("  Analog:  NTC Thermistor (A0, 10K, Beta=3950)\r\n");
    printf("  Digital: DS18B20 (Pin 2, OneWire, 10-bit)\r\n");
    printf("CONDITIONING PIPELINE:\r\n");
    printf("  1. Saturation: [");
    char satMinStr[8], satMaxStr[8];
    dtostrf(SATURATION_MIN, 4, 1, satMinStr);
    dtostrf(SATURATION_MAX, 5, 1, satMaxStr);
    printf("%s, %s] C\r\n", satMinStr, satMaxStr);
    printf("  2. Median Filter: %u samples\r\n",
           (unsigned int)MEDIAN_WINDOW_SIZE);
    printf("  3. EWMA Alpha: ");
    char alphaStr[6];
    dtostrf(EWMA_ALPHA, 3, 1, alphaStr);
    printf("%s\r\n", alphaStr);
    printf("THRESHOLDS:\r\n");
    char ahBuf[8], alBuf[8];
    dtostrf(ANALOG_THRESHOLD_HIGH, 4, 1, ahBuf);
    dtostrf(ANALOG_THRESHOLD_LOW,  4, 1, alBuf);
    printf("  HIGH=%sC  LOW=%sC\r\n", ahBuf, alBuf);
    printf("  Debounce: %u confirmations\r\n",
           (unsigned int)ALERT_DEBOUNCE_COUNT);
    printf("LEDs:\r\n");
    printf("  GREEN  = system normal (no alerts)\r\n");
    printf("  RED    = analog sensor alert\r\n");
    printf("  YELLOW = digital sensor alert\r\n");
    printf("TIMING:\r\n");
    printf("  Acquisition:    %u ms\r\n",
           (unsigned int)TASK_ACQUISITION_PERIOD_MS);
    printf("  Display/LCD:    %u ms\r\n",
           (unsigned int)TASK_DISPLAY_PERIOD_MS);
    printf("  STDIO Report:   every 2 seconds\r\n");
    printf("================================================\r\n\r\n");

    // ── Create synchronization primitives ────────────────────────────────
    sensorDataInit();

    // ── Create FreeRTOS tasks ────────────────────────────────────────────
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

void lab3_2Loop() {
    // All application logic runs inside the three FreeRTOS tasks.
    // This function intentionally does nothing.
}
