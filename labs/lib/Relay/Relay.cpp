/**
 * @file Relay.cpp
 * @brief Binary Relay Actuator Driver Implementation
 */

#include "Relay.h"

Relay::Relay(uint8_t pin, bool activeHigh)
    : _pin(pin), _activeHigh(activeHigh), _state(false) {}

void Relay::init() {
    pinMode(_pin, OUTPUT);
    turnOff();
}

void Relay::turnOn() {
    _state = true;
    digitalWrite(_pin, _activeHigh ? HIGH : LOW);
}

void Relay::turnOff() {
    _state = false;
    digitalWrite(_pin, _activeHigh ? LOW : HIGH);
}

void Relay::toggle() {
    if (_state) turnOff();
    else turnOn();
}

void Relay::setState(bool on) {
    if (on) turnOn();
    else turnOff();
}

bool Relay::isOn() const {
    return _state;
}
