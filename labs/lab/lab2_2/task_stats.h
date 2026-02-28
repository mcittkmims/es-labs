/**
 * @file task_stats.h
 * @brief Lab 2.2 — Task 2: Statistics Update and Yellow LED Blink Sequencer
 *
 * Declares the FreeRTOS task function for Task 2, which waits for press
 * events via a binary semaphore, updates cumulative statistics under mutex
 * protection, and drives a yellow LED blink sequence.
 */

#ifndef TASK_STATS_H
#define TASK_STATS_H

/**
 * @brief FreeRTOS task function — Statistics and yellow LED blink.
 *
 * Blocks on the binary semaphore (xNewPressSemaphore) until Task 1 signals
 * a new press event. When woken:
 *   1. Acquires xSharedDataMutex to read g_pressInfo and update g_stats.
 *   2. Determines the blink count: 5 blinks for short press, 10 for long.
 *   3. Drives the yellow LED blink sequence using vTaskDelay() — a clean
 *      blocking pattern that is only possible with preemptive scheduling.
 *      In the bare-metal version, this required a non-blocking state machine.
 *
 * If another press occurs during the blink sequence, the binary semaphore
 * latches the event and Task 2 processes it after completing the current
 * sequence. This is acceptable because rapid consecutive presses are
 * relatively infrequent in normal use.
 *
 * @param pvParameters Unused (NULL).
 */
void vTaskStats(void *pvParameters);

#endif // TASK_STATS_H
