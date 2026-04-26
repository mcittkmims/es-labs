/**
 * @file task_actuation.cpp
 * @brief Lab 5.1 relay actuation task implementation.
 */

#include "task_actuation.h"
#include "lab5_1_config.h"
#include "shared_state.h"
#include "Relay.h"

#include <Arduino_FreeRTOS.h>

static Relay s_relay(PIN_RELAY, RELAY_ACTIVE_HIGH);

void vTaskLab5Actuation(void *pvParameters) {
    (void)pvParameters;

    s_relay.init();

    lab5StateLock();
    lab5StateGet()->actuatorOn = s_relay.isOn();
    lab5StateUnlock();

    for (;;) {
        if (xSemaphoreTake(xLab5ActuatorSemaphore, portMAX_DELAY) != pdTRUE) {
            continue;
        }

        lab5StateLock();
        bool commandOn = lab5StateGet()->controlCommandOn;
        bool previousOn = lab5StateGet()->actuatorOn;
        lab5StateUnlock();

        s_relay.setState(commandOn);

        lab5StateLock();
        Lab5ControlState *state = lab5StateGet();
        state->actuatorOn = s_relay.isOn();
        if (state->actuatorOn != previousOn) {
            state->actuatorSwitches++;
        }
        lab5StateUnlock();
    }
}
