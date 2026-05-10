/**
 * @file Button.cpp
 * @brief Push-Button Driver Implementation
 *
 * Implements the Button class declared in Button.h. The debounce algorithm
 * is the classical "stable-time" filter:
 *
 *   1. Each call to update() reads the raw logical level of the pin.
 *   2. When the raw level differs from the previous sample, a timestamp
 *      is recorded but no edge is emitted yet.
 *   3. If the new raw level remains stable for at least @c _debounceMs,
 *      it becomes the new "clean" state and the corresponding rising or
 *      falling edge flag is armed for the application to consume.
 *
 * The driver intentionally encapsulates @c pinMode and @c digitalRead so
 * application/lab modules never call these functions directly.
 */

#include "Button.h"

Button::Button(uint8_t pin, bool activeLow, uint16_t debounceMs)
    : _pin(pin),
      _activeLow(activeLow),
      _debounceMs(debounceMs),
      _stableState(false),
      _lastRawState(false),
      _lastChangeMs(0),
      _pressedFlag(false),
      _releasedFlag(false) {}

void Button::init() {
    // Configure the pin direction and pull-up according to the wiring.
    if (_activeLow) {
        pinMode(_pin, INPUT_PULLUP);
    } else {
        pinMode(_pin, INPUT);
    }

    // Take an initial reading so the first update() does not generate a
    // spurious edge from an uninitialised "previous" state.
    _lastRawState  = readLogical();
    _stableState   = _lastRawState;
    _lastChangeMs  = millis();
    _pressedFlag   = false;
    _releasedFlag  = false;
}

void Button::update() {
    bool raw = readLogical();
    uint32_t now = millis();

    // Detect a transition in the raw reading and (re)arm the debounce timer.
    if (raw != _lastRawState) {
        _lastRawState = raw;
        _lastChangeMs = now;
        return;
    }

    // The raw level has been stable; commit it as the new debounced state
    // once the configured stable window has elapsed.
    if (raw != _stableState && (now - _lastChangeMs) >= _debounceMs) {
        _stableState = raw;
        if (_stableState) {
            _pressedFlag = true;     // rising edge: idle -> pressed
        } else {
            _releasedFlag = true;    // falling edge: pressed -> idle
        }
    }
}

bool Button::isPressed() const {
    return _stableState;
}

bool Button::wasPressed() {
    bool flag = _pressedFlag;
    _pressedFlag = false;            // one-shot: consume the event
    return flag;
}

bool Button::wasReleased() {
    bool flag = _releasedFlag;
    _releasedFlag = false;
    return flag;
}

bool Button::readLogical() const {
    int level = digitalRead(_pin);
    // Convert the electrical level into a logical "pressed = true" boolean
    // based on the configured polarity.
    return _activeLow ? (level == LOW) : (level == HIGH);
}
