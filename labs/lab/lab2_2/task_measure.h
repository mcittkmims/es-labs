/**
 * @file task_measure.h
 * @brief Lab 2.2 — Task 1: Button Detection and Duration Measurement
 *
 * Declares the FreeRTOS task function for Task 1, which monitors a push
 * button using a debounce FSM, measures press duration, and provides
 * visual feedback through green/red indicator LEDs.
 */

#ifndef TASK_MEASURE_H
#define TASK_MEASURE_H

/**
 * @brief FreeRTOS task function — Button detection and LED signaling.
 *
 * Runs periodically every 10 ms using vTaskDelay(). On each iteration:
 *   1. Reads the button pin and advances a four-state debounce FSM.
 *   2. When a complete press cycle is detected (press + release confirmed),
 *      writes the duration and type into g_pressInfo under mutex protection,
 *      then gives the binary semaphore to wake Task 2.
 *   3. Lights the green LED (short press) or red LED (long press) for
 *      LED_INDICATOR_DURATION_MS, with automatic turn-off managed via
 *      a timestamp check on each tick.
 *
 * @param pvParameters Unused (NULL).
 */
void vTaskMeasure(void *pvParameters);

#endif // TASK_MEASURE_H
