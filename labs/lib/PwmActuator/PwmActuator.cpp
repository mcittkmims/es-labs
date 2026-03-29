/**
 * @file PwmActuator.cpp
 * @brief PWM Analog Actuator Driver Implementation
 *
 * Converts a percentage duty cycle to a 0--255 PWM value and
 * writes it to the configured pin using Arduino's analogWrite().
 */

#include "PwmActuator.h"

PwmActuator::PwmActuator(uint8_t pin)
    : _pin(pin), _dutyCycle(0.0f) {}

void PwmActuator::init() {
    pinMode(_pin, OUTPUT);
    analogWrite(_pin, 0);
    _dutyCycle = 0.0f;
}

void PwmActuator::setDutyCycle(float percent) {
    // Clamp to valid range
    if (percent < 0.0f) percent = 0.0f;
    if (percent > 100.0f) percent = 100.0f;
    _dutyCycle = percent;

    // Convert percentage to 0-255 PWM value
    uint8_t pwmVal = (uint8_t)(percent * 255.0f / 100.0f);
    analogWrite(_pin, pwmVal);
}

float PwmActuator::getDutyCycle() const {
    return _dutyCycle;
}

uint8_t PwmActuator::getRawPwm() const {
    return (uint8_t)(_dutyCycle * 255.0f / 100.0f);
}
