/**
 * @file Button.h
 * @brief Push-Button Driver Interface (with software debounce)
 *
 * Provides a reusable abstraction for a momentary push button connected
 * to a digital GPIO pin. The driver implements a small internal finite
 * state machine that suppresses contact bounce by requiring the input
 * level to remain stable for a configurable debounce window before a
 * transition is accepted.
 *
 * The class supports two electrical configurations:
 *   - Active-LOW (button shorts the pin to GND, INPUT_PULLUP enabled).
 *     This is the default and the most common breadboard wiring.
 *   - Active-HIGH (external pull-down resistor; button drives the pin
 *     to VCC), selected via the optional constructor parameter.
 *
 * Typical usage in an application loop:
 * @code
 *   Button btn(7);
 *   btn.init();
 *   while (true) {
 *       btn.update();
 *       if (btn.wasPressed()) {
 *           // handle a confirmed press edge
 *       }
 *   }
 * @endcode
 */

#ifndef BUTTON_H
#define BUTTON_H

#include <Arduino.h>

/**
 * @class Button
 * @brief Debounced push-button reader.
 *
 * Hardware access (pin configuration, digital reads) is fully encapsulated
 * inside the driver. The application interacts only with high-level events:
 * "is the button currently pressed?" and "was a clean press edge detected
 * since the last call?".
 */
class Button {
public:
    /**
     * @brief Construct a new Button driver.
     * @param pin           GPIO pin number wired to the button.
     * @param activeLow     true  -> pin reads LOW when pressed (uses INPUT_PULLUP).
     *                      false -> pin reads HIGH when pressed (uses INPUT).
     * @param debounceMs    Stable-time threshold (in milliseconds) before a
     *                      logical level change is accepted as a real edge.
     */
    Button(uint8_t pin, bool activeLow = true, uint16_t debounceMs = 30);

    /**
     * @brief Configure the GPIO pin and reset the internal debounce state.
     *
     * Must be called once during application setup, after Serial/STDIO
     * initialization but before the first call to @ref update().
     */
    void init();

    /**
     * @brief Sample the input pin and advance the debounce state machine.
     *
     * Should be called periodically from the main loop or a scheduler tick
     * (e.g. every 5--10 ms). When the input has been stable for at least
     * @p debounceMs, the internally stored "clean" state is updated and
     * the @ref wasPressed() / @ref wasReleased() one-shot flags are armed
     * on the corresponding edges.
     */
    void update();

    /**
     * @brief Query the debounced (stable) button state.
     * @return true if the button is currently held pressed.
     */
    bool isPressed() const;

    /**
     * @brief One-shot rising-edge query (idle -> pressed).
     *
     * Returns true exactly once per confirmed press: the flag is consumed
     * by the call. Designed to be polled directly from an FSM driver.
     *
     * @return true if a clean press edge was detected since the last call.
     */
    bool wasPressed();

    /**
     * @brief One-shot falling-edge query (pressed -> idle).
     * @return true if a clean release edge was detected since the last call.
     */
    bool wasReleased();

private:
    uint8_t  _pin;             ///< GPIO pin number
    bool     _activeLow;       ///< true if pressed = LOW, false if pressed = HIGH
    uint16_t _debounceMs;      ///< Debounce window in milliseconds

    bool     _stableState;     ///< Last accepted (debounced) logical state (true = pressed)
    bool     _lastRawState;    ///< Last raw sample (logical, post-polarity correction)
    uint32_t _lastChangeMs;    ///< Timestamp of the last raw transition

    bool     _pressedFlag;     ///< Latched rising edge, cleared by wasPressed()
    bool     _releasedFlag;    ///< Latched falling edge, cleared by wasReleased()

    /**
     * @brief Read the pin and convert the electrical level into a logical
     *        "pressed/not-pressed" boolean according to the polarity setting.
     */
    bool readLogical() const;
};

#endif // BUTTON_H
