/**
 * @file TaskScheduler.h
 * @brief Non-Preemptive Bare-Metal Task Scheduler
 *
 * Implements a cooperative, non-preemptive task scheduler suitable for
 * bare-metal embedded applications. Each task is described by a TaskContext_t
 * structure that holds a function pointer, a recurrence period, and a startup
 * offset. The scheduler maintains a flat array of these contexts and, on each
 * call to schedulerRun(), executes at most one due task — the one with the
 * earliest scheduled deadline.
 *
 * This design satisfies the requirement of "one task active per tick" while
 * providing deterministic, offset-controlled startup sequencing.
 *
 * Usage:
 *   TaskContext_t tasks[] = {
 *       { task1Func, 10,    0  },
 *       { task2Func, 50,    5  },
 *       { task3Func, 10000, 0  },
 *   };
 *   schedulerInit(tasks, 3);     // Called once in setup()
 *   schedulerRun(tasks, 3);      // Called every loop() iteration
 */

#ifndef TASK_SCHEDULER_H
#define TASK_SCHEDULER_H

#include <Arduino.h>

/**
 * @brief Task context structure.
 *
 * Holds all scheduling metadata for a single task. An array of these
 * structs is passed to the scheduler to define a set of periodic tasks.
 *
 * Members are filled by the caller (funcPtr, period, offset); nextRun is
 * computed internally by schedulerInit().
 */
typedef struct {
    void     (*funcPtr)();  /**< Pointer to the task function. Must be non-blocking. */
    uint32_t  period;       /**< Recurrence period in milliseconds. */
    uint32_t  offset;       /**< Startup offset in milliseconds before first execution. */
    uint32_t  nextRun;      /**< Absolute time (ms) of the next scheduled execution. Managed by scheduler. */
} TaskContext_t;

/**
 * @brief Initialize the scheduler.
 *
 * Computes the initial nextRun time for each task as millis() + offset.
 * Must be called once from setup() before the first schedulerRun() call.
 *
 * @param tasks  Pointer to the task context array.
 * @param count  Number of tasks in the array.
 */
void schedulerInit(TaskContext_t *tasks, uint8_t count);

/**
 * @brief Run one scheduler tick.
 *
 * Scans the task array for all tasks whose nextRun deadline has passed.
 * Among due tasks, it selects the one with the earliest missed deadline
 * (highest priority) and executes it once. After execution, the task's
 * nextRun is advanced by its period. If execution falls far behind,
 * the scheduler re-anchors nextRun to avoid burst catch-up.
 *
 * Exactly one task runs per call; if no task is due, the function returns
 * immediately without executing anything.
 *
 * @param tasks  Pointer to the task context array.
 * @param count  Number of tasks in the array.
 */
void schedulerRun(TaskContext_t *tasks, uint8_t count);

#endif // TASK_SCHEDULER_H
