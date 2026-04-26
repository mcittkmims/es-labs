/**
 * @file task_acquisition.cpp
 * @brief Lab 5.1 DHT11 and potentiometer acquisition task.
 */

#include "task_acquisition.h"
#include "lab5_1_config.h"
#include "shared_state.h"

#include "DhtSensor.h"
#include "AnalogSetpointInput.h"

#include <Arduino_FreeRTOS.h>
#include <DHT.h>

static DhtSensor s_dht(PIN_DHT22, DHT11);
static AnalogSetpointInput s_setpointPot(
    PIN_SETPOINT_POT,
    SETPOINT_MIN_C,
    SETPOINT_MAX_C
);

void vTaskLab5Acquisition(void *pvParameters) {
    (void)pvParameters;

    s_dht.init();
    s_setpointPot.init();

    TickType_t lastWake = xTaskGetTickCount();
    const TickType_t period = pdMS_TO_TICKS(TASK_ACQUISITION_PERIOD_MS);

    for (;;) {
        bool sensorOk = s_dht.read();
        float temperature = s_dht.getTemperatureC();
        float humidity = s_dht.getHumidityPercent();
        float potSetpoint = s_setpointPot.readValue();
        uint16_t potRaw = s_setpointPot.getLastRaw();

        lab5StateLock();
        Lab5ControlState *state = lab5StateGet();

        state->sensorValid = sensorOk;
        state->measuredTempC = temperature;
        state->measuredHumidityPercent = humidity;
        state->potRaw = potRaw;
        state->potSetpointC = potSetpoint;

        if (state->setpointSource == SETPOINT_SOURCE_POT) {
            state->activeSetpointC = potSetpoint;
        } else {
            state->activeSetpointC = state->manualSetpointC;
        }

        state->sampleCount++;
        state->lastSampleTick = xTaskGetTickCount();

        lab5StateUnlock();
        xSemaphoreGive(xLab5NewSampleSemaphore);

        vTaskDelayUntil(&lastWake, period);
    }
}
