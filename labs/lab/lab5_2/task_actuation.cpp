/**
 * @file task_actuation.cpp
 * @brief Lab 5.2 fan actuation task implementation.
 */

#include "task_actuation.h"
#include "lab5_2_config.h"
#include "shared_state.h"
#include "HBridgeMotor.h"

#include <Arduino_FreeRTOS.h>

static HBridgeMotor s_fan(PIN_FAN_PWM, PIN_FAN_IN1, PIN_FAN_IN2);

static float mapPidOutputToFanDuty(float outputPercent) {
    if (outputPercent <= FAN_STOP_THRESHOLD_PERCENT) {
        return 0.0f;
    }

    float usableRange = 100.0f - FAN_MIN_RUNNING_DUTY_PERCENT;
    return FAN_MIN_RUNNING_DUTY_PERCENT +
           (outputPercent * usableRange / 100.0f);
}

void vTaskLab5PidActuation(void *pvParameters) {
    (void)pvParameters;

    s_fan.init();

    lab5PidStateLock();
    lab5PidStateGet()->appliedDutyPercent = s_fan.getDuty();
    lab5PidStateGet()->fanRunning = false;
    lab5PidStateUnlock();

    for (;;) {
        if (xSemaphoreTake(xLab5PidActuatorSemaphore, portMAX_DELAY) != pdTRUE) {
            continue;
        }

        lab5PidStateLock();
        Lab5PidState *state = lab5PidStateGet();
        float outputPercent = state->controlOutputPercent;
        bool sensorValid = state->sensorValid;
        lab5PidStateUnlock();

        float dutyPercent = sensorValid ? mapPidOutputToFanDuty(outputPercent) : 0.0f;
        if (dutyPercent > 0.0f) {
            s_fan.setForward(dutyPercent);
        } else {
            s_fan.stop();
        }

        lab5PidStateLock();
        state = lab5PidStateGet();
        state->appliedDutyPercent = s_fan.getDuty();
        state->fanRunning = s_fan.getDuty() > 0.0f;
        state->actuatorUpdates++;
        lab5PidStateUnlock();
    }
}
