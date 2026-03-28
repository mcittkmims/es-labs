/**
 * @file ActuatorControl.cpp
 * @brief Binary Actuator Controller Implementation
 *
 * Implements the ActuatorControl class which wraps the Led GPIO driver
 * with a BinaryConditioner pipeline. On each update() call the latest
 * raw command is processed through the conditioner; if the conditioned
 * output differs from the current hardware state, the GPIO is updated.
 */

#include "ActuatorControl.h"

// ── Constructor ───────────────────────────────────────────────────────────────

ActuatorControl::ActuatorControl(uint8_t pin,
                                 uint32_t debouncePeriodMs,
                                 uint8_t persistCount)
    : _led(pin),
      _conditioner(debouncePeriodMs, persistCount),
      _rawCommand(State::OFF),
      _currentState(State::OFF)
{
}

// ── Initialization ────────────────────────────────────────────────────────────

void ActuatorControl::init() {
    _led.init();
    _led.turnOff();
    _currentState = State::OFF;
    _rawCommand   = State::OFF;
}

// ── Command input ─────────────────────────────────────────────────────────────

void ActuatorControl::setRawCommand(State cmd) {
    _rawCommand = cmd;
}

// ── Conditioning and hardware update ─────────────────────────────────────────

void ActuatorControl::update() {
    // Feed the latest raw command into the signal conditioning pipeline
    bool rawBool     = (_rawCommand == State::ON);
    bool conditioned = _conditioner.process(rawBool);

    // Drive hardware output if the conditioned state differs from current
    State newState = conditioned ? State::ON : State::OFF;
    if (newState != _currentState) {
        _currentState = newState;
        if (_currentState == State::ON) {
            _led.turnOn();
        } else {
            _led.turnOff();
        }
    }
}

// ── Accessors ─────────────────────────────────────────────────────────────────

ActuatorControl::State ActuatorControl::getState()      const { return _currentState; }
ActuatorControl::State ActuatorControl::getRawCommand() const { return _rawCommand; }
bool    ActuatorControl::isDebouncing()                 const { return _conditioner.isDebouncing(); }
uint8_t ActuatorControl::getConfirmCount()              const { return _conditioner.getConfirmCount(); }
uint8_t ActuatorControl::getRequiredConfirmCount()      const { return _conditioner.getRequiredConfirmCount(); }
