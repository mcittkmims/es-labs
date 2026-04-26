/**
 * @file OnOffHysteresisController.cpp
 * @brief Heating-style ON-OFF controller with hysteresis implementation.
 */

#include "OnOffHysteresisController.h"

OnOffHysteresisController::OnOffHysteresisController(float setpoint,
                                                     float hysteresisBand)
    : _setpoint(setpoint),
      _hysteresisBand(hysteresisBand),
      _state(HYSTERESIS_OUTPUT_OFF) {}

void OnOffHysteresisController::init() {
    _state = HYSTERESIS_OUTPUT_OFF;
}

void OnOffHysteresisController::setConfig(float setpoint, float hysteresisBand) {
    _setpoint = setpoint;
    _hysteresisBand = hysteresisBand;
}

bool OnOffHysteresisController::update(float measuredValue) {
    if (_state == HYSTERESIS_OUTPUT_OFF &&
        measuredValue <= getLowerThreshold()) {
        _state = HYSTERESIS_OUTPUT_ON;
    } else if (_state == HYSTERESIS_OUTPUT_ON &&
               measuredValue >= getUpperThreshold()) {
        _state = HYSTERESIS_OUTPUT_OFF;
    }

    return isOutputOn();
}

void OnOffHysteresisController::forceOutput(bool outputOn) {
    _state = outputOn ? HYSTERESIS_OUTPUT_ON : HYSTERESIS_OUTPUT_OFF;
}

bool OnOffHysteresisController::isOutputOn() const {
    return _state == HYSTERESIS_OUTPUT_ON;
}

HysteresisOutputState OnOffHysteresisController::getState() const {
    return _state;
}

float OnOffHysteresisController::getLowerThreshold() const {
    return _setpoint - (_hysteresisBand * 0.5f);
}

float OnOffHysteresisController::getUpperThreshold() const {
    return _setpoint + (_hysteresisBand * 0.5f);
}

float OnOffHysteresisController::getSetpoint() const {
    return _setpoint;
}

float OnOffHysteresisController::getHysteresisBand() const {
    return _hysteresisBand;
}
