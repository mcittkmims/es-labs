/**
 * @file KeypadInput.h
 * @brief 4x4 Matrix Keypad Driver Interface
 *
 * Provides a reusable abstraction for a 4x4 membrane matrix keypad.
 * Wraps the Keypad library and includes built-in debouncing.
 *
 * The standard 4x4 layout is:
 *   1 2 3 A
 *   4 5 6 B
 *   7 8 9 C
 *   * 0 # D
 *
 * Usage:
 *   byte rows[] = {22, 23, 24, 25};
 *   byte cols[] = {26, 27, 28, 29};
 *   KeypadInput kp(rows, cols);
 *   kp.init();
 *   char key = kp.getKey(); // 0 if no key pressed
 */

#ifndef KEYPAD_INPUT_H
#define KEYPAD_INPUT_H

#include <Arduino.h>
#include <Keypad.h>

/// Number of rows in the 4x4 matrix keypad
static const byte KEYPAD_ROWS = 4;

/// Number of columns in the 4x4 matrix keypad
static const byte KEYPAD_COLS = 4;

/**
 * @class KeypadInput
 * @brief Controls a 4x4 matrix keypad with debouncing.
 *
 * Encapsulates a standard 4x4 membrane keypad using the Keypad library.
 * The key layout is fixed to the standard telephone/security keypad layout.
 * Pin assignments are configurable via the constructor.
 */
class KeypadInput {
public:
    /**
     * @brief Construct a new KeypadInput object.
     * @param rowPins Array of 4 GPIO pin numbers for keypad rows.
     *                The array must remain valid for the object's lifetime.
     * @param colPins Array of 4 GPIO pin numbers for keypad columns.
     *                The array must remain valid for the object's lifetime.
     */
    KeypadInput(byte *rowPins, byte *colPins);

    /**
     * @brief Initialize the keypad with debounce settings.
     *
     * Sets the debounce time to 20ms for reliable membrane keypad operation.
     */
    void init();

    /**
     * @brief Read a key press from the keypad (non-blocking).
     *
     * Scans the key matrix and returns the pressed key character.
     * Returns 0 (null character) if no key is currently pressed.
     *
     * @return The character of the pressed key, or 0 if none.
     */
    char getKey();

private:
    Keypad _keypad;  ///< Underlying Keypad library instance
};

#endif // KEYPAD_INPUT_H
