/**
 * @file Led.cpp
 * @brief LED Driver Implementation
 *
 * Implements the Led class methods for controlling an LED
 * via digital GPIO. Uses Arduino's digitalWrite for hardware control.
 */

#include "Led.h"

Led::Led(uint8_t pin) : ledPin(pin), state(false) {}

void Led::init() {
    pinMode(ledPin, OUTPUT);
    digitalWrite(ledPin, LOW);
    state = false;
}

void Led::turnOn() {
    digitalWrite(ledPin, HIGH);
    state = true;
}

void Led::turnOff() {
    digitalWrite(ledPin, LOW);
    state = false;
}

void Led::toggle() {
    if (state) {
        turnOff();
    } else {
        turnOn();
    }
}

bool Led::isOn() const {
    return state;
}
