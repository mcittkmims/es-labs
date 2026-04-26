/**
 * @file AnalogSetpointInput.cpp
 * @brief Potentiometer-based setpoint input wrapper implementation.
 */

#include "AnalogSetpointInput.h"

AnalogSetpointInput::AnalogSetpointInput(uint8_t adcPin, float minValue,
                                         float maxValue, uint8_t adcResolution)
    : _adcPin(adcPin),
      _minValue(minValue),
      _maxValue(maxValue),
      _adcMax((1U << adcResolution) - 1U),
      _lastRaw(0),
      _lastValue(minValue) {}

void AnalogSetpointInput::init() {
    pinMode(_adcPin, INPUT);
    _lastRaw = 0;
    _lastValue = _minValue;
}

uint16_t AnalogSetpointInput::readRaw() {
    _lastRaw = analogRead(_adcPin);
    return _lastRaw;
}

float AnalogSetpointInput::readValue() {
    uint16_t raw = readRaw();
    float ratio = (float)raw / (float)_adcMax;
    _lastValue = _minValue + ratio * (_maxValue - _minValue);
    return _lastValue;
}

void AnalogSetpointInput::setRange(float minValue, float maxValue) {
    _minValue = minValue;
    _maxValue = maxValue;
}

uint16_t AnalogSetpointInput::getLastRaw() const {
    return _lastRaw;
}

float AnalogSetpointInput::getLastValue() const {
    return _lastValue;
}
