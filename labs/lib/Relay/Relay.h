/**
 * @file Relay.h
 * @brief Binary Relay Actuator Driver Interface
 *
 * Provides a reusable abstraction for controlling a relay module
 * connected to a digital GPIO pin. Supports ON/OFF control with
 * state tracking and configurable active level (HIGH or LOW).
 *
 * Usage:
 *   Relay relay(7, true);  // pin 7, active-HIGH
 *   relay.init();
 *   relay.turnOn();
 */

#ifndef RELAY_H
#define RELAY_H

#include <Arduino.h>

/**
 * @class Relay
 * @brief Controls a relay module on a specified GPIO pin.
 */
class Relay {
public:
    /**
     * @brief Construct a new Relay object.
     * @param pin GPIO pin connected to the relay control input.
     * @param activeHigh True if relay activates on HIGH (default).
     */
    Relay(uint8_t pin, bool activeHigh = true);

    /** @brief Initialize the relay pin as OUTPUT, set to OFF. */
    void init();

    /** @brief Activate the relay (close the circuit). */
    void turnOn();

    /** @brief Deactivate the relay (open the circuit). */
    void turnOff();

    /** @brief Toggle the relay state. */
    void toggle();

    /**
     * @brief Set relay state directly.
     * @param on True to activate, false to deactivate.
     */
    void setState(bool on);

    /** @brief Check if relay is currently activated. */
    bool isOn() const;

private:
    uint8_t _pin;
    bool    _activeHigh;
    bool    _state;
};

#endif // RELAY_H
