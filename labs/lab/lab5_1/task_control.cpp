/**
 * @file task_control.cpp
 * @brief Lab 5.1 ON-OFF hysteresis control task implementation.
 */

#include "task_control.h"
#include "lab5_1_config.h"
#include "shared_state.h"
#include "OnOffHysteresisController.h"

#include <Arduino_FreeRTOS.h>
#include <math.h>

static OnOffHysteresisController s_controller(
    SETPOINT_DEFAULT_C,
    HYSTERESIS_DEFAULT_C
);

void vTaskLab5Control(void *pvParameters) {
    (void)pvParameters;

    s_controller.init();

    for (;;) {
        if (xSemaphoreTake(xLab5NewSampleSemaphore, portMAX_DELAY) != pdTRUE) {
            continue;
        }

        lab5StateLock();
        Lab5ControlState *state = lab5StateGet();
        float temperature = state->measuredTempC;
        float setpoint = state->activeSetpointC;
        float hysteresis = state->hysteresisBandC;
        bool valid = state->sensorValid && !isnan(temperature);
        lab5StateUnlock();

        s_controller.setConfig(setpoint, hysteresis);

        bool commandOn = false;
        if (valid) {
            commandOn = s_controller.update(temperature);
        } else {
            // Fail safe: if the sensor reading is invalid, de-energize relay.
            s_controller.forceOutput(false);
            commandOn = false;
        }

        lab5StateLock();
        state = lab5StateGet();
        state->controlCommandOn = commandOn;
        state->controllerState = s_controller.getState();
        state->lowerThresholdC = s_controller.getLowerThreshold();
        state->upperThresholdC = s_controller.getUpperThreshold();
        state->controlCycles++;
        lab5StateUnlock();

        xSemaphoreGive(xLab5ActuatorSemaphore);
    }
}
