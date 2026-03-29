/**
 * @file task_control.cpp
 * @brief Lab 4 — Actuator Control & Conditioning Task Implementation
 *
 * Runs at 100ms period. Applies debouncing to the binary relay command
 * to prevent false toggles. Processes the analog PWM command through
 * a 4-stage conditioning pipeline (saturate -> median -> EWMA -> ramp)
 * before applying it to the PWM output. Evaluates an overload threshold
 * alert FSM on the conditioned analog value.
 */

#include "task_control.h"
#include "shared_state.h"
#include "lab4_config.h"

#include "Relay.h"
#include "PwmActuator.h"
#include "ActuatorConditioner.h"
#include "ThresholdAlert.h"
#include "Led.h"
#include <stdio.h>

// Hardware instances
static Relay relay(PIN_RELAY, true);
static PwmActuator pwmAct(PIN_PWM_ACT);
static Led ledGreen(PIN_LED_GREEN);
static Led ledRed(PIN_LED_RED);

// Signal conditioning for analog actuator
static ActuatorConditioner conditioner(
    ACT_MEDIAN_WINDOW, ACT_EWMA_ALPHA,
    ACT_MIN_CLAMP, ACT_MAX_CLAMP,
    ACT_RAMP_STEP
);

// Overload alert FSM
static ThresholdAlert overloadAlert(
    OVERLOAD_THRESHOLD_HIGH, OVERLOAD_THRESHOLD_LOW,
    OVERLOAD_DEBOUNCE_COUNT
);

// Binary actuator debounce state
static bool lastRelayCmd = false;
static uint8_t relayDebounceCounter = 0;

void vTaskControl(void *pvParameters) {
    (void)pvParameters;

    relay.init();
    pwmAct.init();
    ledGreen.init();
    ledRed.init();
    overloadAlert.init();
    ledGreen.turnOn();

    TickType_t xLastWake = xTaskGetTickCount();
    const TickType_t xPeriod = pdMS_TO_TICKS(TASK_CONTROL_PERIOD_MS);

    for (;;) {
        sharedStateLock();
        ActuatorState *s = sharedStateGet();

        // ── Binary actuator: debounced relay control ────────────────
        if (s->relayCommandOn == lastRelayCmd) {
            if (relayDebounceCounter < RELAY_DEBOUNCE_COUNT) {
                relayDebounceCounter++;
            }
        } else {
            relayDebounceCounter = 0;
            lastRelayCmd = s->relayCommandOn;
        }

        if (relayDebounceCounter >= RELAY_DEBOUNCE_COUNT) {
            relay.setState(lastRelayCmd);
        }
        s->relayActualOn = relay.isOn();
        s->relayDebounceCount = relayDebounceCounter;

        // ── Analog actuator: conditioning pipeline ──────────────────
        float rawCmd = s->pwmCommandPercent;
        float output = conditioner.process(rawCmd);
        pwmAct.setDuty(output);

        s->pwmConditioned = conditioner.getConditionedTarget();
        s->pwmRamped = conditioner.getRampedOutput();
        s->pwmRawValue = pwmAct.getRawPwm();

        // ── Overload alert evaluation ───────────────────────────────
        overloadAlert.update(output);
        s->overloadAlert = overloadAlert.isAlertActive();

        // ── LED indicators ──────────────────────────────────────────
        if (s->overloadAlert) {
            ledRed.turnOn();
            ledGreen.turnOff();
        } else {
            ledRed.turnOff();
            ledGreen.turnOn();
        }

        sharedStateUnlock();
        vTaskDelayUntil(&xLastWake, xPeriod);
    }
}
