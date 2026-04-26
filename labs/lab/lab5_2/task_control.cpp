/**
 * @file task_control.cpp
 * @brief Lab 5.2 PID control task implementation.
 */

#include "task_control.h"
#include "lab5_2_config.h"
#include "shared_state.h"
#include "PidController.h"

#include <Arduino_FreeRTOS.h>
#include <math.h>

static PidController s_pid(
    PID_DEFAULT_KP,
    PID_DEFAULT_KI,
    PID_DEFAULT_KD,
    PID_OUTPUT_MIN_PERCENT,
    PID_OUTPUT_MAX_PERCENT,
    PID_REVERSE
);

static float elapsedSeconds(TickType_t previousTick, TickType_t currentTick) {
    if (previousTick == 0 || currentTick <= previousTick) {
        return (float)TASK_ACQUISITION_PERIOD_MS / 1000.0f;
    }

    TickType_t deltaTicks = currentTick - previousTick;
    return ((float)deltaTicks * (float)portTICK_PERIOD_MS) / 1000.0f;
}

void vTaskLab5PidControl(void *pvParameters) {
    (void)pvParameters;

    s_pid.init();
    TickType_t previousControlTick = 0;

    for (;;) {
        if (xSemaphoreTake(xLab5PidNewSampleSemaphore, portMAX_DELAY) != pdTRUE) {
            continue;
        }

        TickType_t now = xTaskGetTickCount();
        float dtSeconds = elapsedSeconds(previousControlTick, now);
        previousControlTick = now;

        lab5PidStateLock();
        Lab5PidState *state = lab5PidStateGet();
        float temperature = state->measuredTempC;
        float setpoint = state->activeSetpointC;
        float kp = state->kp;
        float ki = state->ki;
        float kd = state->kd;
        bool valid = state->sensorValid && !isnan(temperature);
        lab5PidStateUnlock();

        s_pid.setTunings(kp, ki, kd);

        float output = 0.0f;
        if (valid) {
            output = s_pid.update(setpoint, temperature, dtSeconds);
        } else {
            s_pid.reset();
        }

        lab5PidStateLock();
        state = lab5PidStateGet();
        state->controlOutputPercent = output;
        state->errorC = valid ? s_pid.getError() : 0.0f;
        state->pidIntegral = s_pid.getIntegral();
        state->pidDerivative = s_pid.getDerivative();
        state->controlCycles++;
        lab5PidStateUnlock();

        xSemaphoreGive(xLab5PidActuatorSemaphore);
    }
}
