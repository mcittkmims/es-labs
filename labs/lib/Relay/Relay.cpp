/**
 * @file Relay.cpp
 * @brief Relay Driver Implementation
 *
 * Controls a relay module via a digital GPIO pin. Handles the
 * logic inversion for active-LOW relay modules automatically.
 */

#include "Relay.h"

Relay::Relay(uint8_t pin, bool activeHigh)
    : _pin(pin), _activeHigh(activeHigh), _state(false) {}

void Relay::init() {
    pinMode(_pin, OUTPUT);
    turnOff();
}

void Relay::turnOn() {
    digitalWrite(_pin, _activeHigh ? HIGH : LOW);
    _state = true;
}

void Relay::turnOff() {
    digitalWrite(_pin, _activeHigh ? LOW : HIGH);
    _state = false;
}

void Relay::setState(bool on) {
    if (on) {
        turnOn();
    } else {
        turnOff();
    }
}

bool Relay::isOn() const {
    return _state;
}
