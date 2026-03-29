/**
 * @file PwmActuator.cpp
 * @brief PWM-Based Analog Actuator Driver Implementation
 */

#include "PwmActuator.h"

PwmActuator::PwmActuator(uint8_t pin)
    : _pin(pin), _duty(0.0f) {}

void PwmActuator::init() {
    pinMode(_pin, OUTPUT);
    analogWrite(_pin, 0);
    _duty = 0.0f;
}

void PwmActuator::setDuty(float duty) {
    // Clamp to valid range
    if (duty < 0.0f) duty = 0.0f;
    if (duty > 100.0f) duty = 100.0f;
    _duty = duty;
    uint8_t pwmVal = (uint8_t)(_duty * 255.0f / 100.0f);
    analogWrite(_pin, pwmVal);
}

float PwmActuator::getDuty() const {
    return _duty;
}

uint8_t PwmActuator::getRawPwm() const {
    return (uint8_t)(_duty * 255.0f / 100.0f);
}
