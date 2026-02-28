/**
 * @file shared_state.cpp
 * @brief Lab 2.2 — Shared State Definitions and Synchronization Init
 *
 * Defines the shared data structures and FreeRTOS synchronization
 * primitives used for inter-task communication in the preemptive
 * button press monitoring system.
 */

#include "shared_state.h"

// ──────────────────────────────────────────────────────────────────────────
// Shared data — zero-initialized at startup
// ──────────────────────────────────────────────────────────────────────────

PressInfo_t g_pressInfo = { 0, false };

Stats_t g_stats = { 0, 0, 0, 0 };

// ──────────────────────────────────────────────────────────────────────────
// Synchronization primitives — created at runtime in sharedStateInit()
// ──────────────────────────────────────────────────────────────────────────

SemaphoreHandle_t xNewPressSemaphore = NULL;
SemaphoreHandle_t xSharedDataMutex   = NULL;

void sharedStateInit() {
    // Binary semaphore starts "empty" (not given) — Task 2 will block
    // on xSemaphoreTake() until Task 1 gives it after a press event.
    xNewPressSemaphore = xSemaphoreCreateBinary();

    // Mutex provides priority inheritance: if Task 3 (low priority) holds
    // the mutex and Task 1 (high priority) tries to acquire it, FreeRTOS
    // temporarily raises Task 3's priority to prevent priority inversion.
    xSharedDataMutex = xSemaphoreCreateMutex();
}
