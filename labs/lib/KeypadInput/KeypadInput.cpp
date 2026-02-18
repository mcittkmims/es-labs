/**
 * @file KeypadInput.cpp
 * @brief 4x4 Matrix Keypad Driver Implementation
 *
 * Implements the KeypadInput class using the Keypad library for
 * matrix scanning and debouncing. The key map is defined as a
 * standard 4x4 layout with digits 0-9, *, #, and A-D.
 */

#include "KeypadInput.h"

/// Standard 4x4 membrane keypad layout
static char keymap[KEYPAD_ROWS][KEYPAD_COLS] = {
    {'1', '2', '3', 'A'},
    {'4', '5', '6', 'B'},
    {'7', '8', '9', 'C'},
    {'*', '0', '#', 'D'}
};

KeypadInput::KeypadInput(byte *rowPins, byte *colPins)
    : _keypad(makeKeymap(keymap), rowPins, colPins,
              KEYPAD_ROWS, KEYPAD_COLS) {}

void KeypadInput::init() {
    // Set debounce time to 20ms for reliable membrane keypad operation.
    // The Keypad library uses an internal state machine that tracks
    // press/release transitions, ensuring clean key detection.
    _keypad.setDebounceTime(20);
}

char KeypadInput::getKey() {
    return _keypad.getKey();
}
