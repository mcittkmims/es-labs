/**
 * @file TaskScheduler.cpp
 * @brief Non-Preemptive Bare-Metal Task Scheduler — Implementation
 *
 * The scheduler follows a Earliest-Deadline-First (EDF) selection policy
 * within a single loop() tick:
 *   1. Scan all tasks to find which ones are due (nextRun <= millis()).
 *   2. Among due tasks, pick the one with the smallest nextRun value
 *      (the most overdue task, meaning highest urgency).
 *   3. Execute that task's function pointer (must be non-blocking).
 *   4. Advance its nextRun by period; if the new nextRun is still in
 *      the past, re-anchor it to now + period to prevent burst catch-up.
 *
 * Since loop() runs at hundreds of kHz on an Arduino Mega 2560 when tasks
 * sleep or have no work, a single task per tick effectively runs all tasks
 * at their correct cadence without starving any of them.
 */

#include "TaskScheduler.h"

void schedulerInit(TaskContext_t *tasks, uint8_t count) {
    uint32_t now = millis();
    for (uint8_t i = 0; i < count; i++) {
        tasks[i].nextRun = now + tasks[i].offset;
    }
}

void schedulerRun(TaskContext_t *tasks, uint8_t count) {
    uint32_t now     = millis();
    int8_t   chosen  = -1;
    uint32_t earliest = UINT32_MAX;

    // Find the most-overdue task among all tasks that are due this tick.
    for (uint8_t i = 0; i < count; i++) {
        if (now >= tasks[i].nextRun) {
            if (tasks[i].nextRun < earliest) {
                earliest = tasks[i].nextRun;
                chosen   = i;
            }
        }
    }

    if (chosen >= 0) {
        // Execute the selected task.
        tasks[chosen].funcPtr();

        // Advance the deadline by one period.
        tasks[chosen].nextRun += tasks[chosen].period;

        // Guard against falling far behind: if the new deadline is still
        // in the past, re-anchor from now to avoid cascading catch-up runs.
        if (tasks[chosen].nextRun < millis()) {
            tasks[chosen].nextRun = millis() + tasks[chosen].period;
        }
    }
}
