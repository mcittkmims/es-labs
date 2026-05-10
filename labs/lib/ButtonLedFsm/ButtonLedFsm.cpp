/**
 * @file ButtonLedFsm.cpp
 * @brief Button-LED Finite State Machine Implementation
 *
 * Implements the two-state Moore automaton declared in ButtonLedFsm.h.
 * The whole behaviour is encoded in the @c _table constant: each row
 * carries the Moore output for the state and the next-state lookup
 * indexed by the input value (0 = no press, 1 = press detected).
 *
 * This mirrors one-to-one the table presented in the laboratory manual
 * (Listing 7.3), so the implementation is a direct realisation of the
 * theoretical model rather than ad-hoc branching.
 */

#include "ButtonLedFsm.h"

// State transition / output table.
// Row index = current state. Column "next[0]" is the next state when the
// input is 0 (no press), "next[1]" when the input is 1 (press detected).
const ButtonLedFsm::StateRow ButtonLedFsm::_table[BUTTON_LED_STATE_COUNT] = {
    // LED_OFF_STATE: output 0; idle if input 0, switch ON if input 1.
    { 0, { LED_OFF_STATE, LED_ON_STATE  } },
    // LED_ON_STATE : output 1; idle if input 0, switch OFF if input 1.
    { 1, { LED_ON_STATE,  LED_OFF_STATE } }
};

ButtonLedFsm::ButtonLedFsm()
    : _state(LED_OFF_STATE), _changed(true) {}

void ButtonLedFsm::init() {
    _state   = LED_OFF_STATE;
    _changed = true;            // request an initial refresh of the output
}

void ButtonLedFsm::processEvent() {
    // The Button driver delivers one-shot, debounced edges, so reaching
    // this function is equivalent to "input = 1" in the table.
    ButtonLedState next = _table[_state].next[1];
    if (next != _state) {
        _state   = next;
        _changed = true;
    }
}

ButtonLedState ButtonLedFsm::getState() const {
    return _state;
}

uint8_t ButtonLedFsm::getOutput() const {
    return _table[_state].output;
}

const char *ButtonLedFsm::getStateName() const {
    return (_state == LED_ON_STATE) ? "ON" : "OFF";
}

bool ButtonLedFsm::changed() const {
    return _changed;
}

void ButtonLedFsm::clearChanged() {
    _changed = false;
}
