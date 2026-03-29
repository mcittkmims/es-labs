/**
 * @file Relay.h
 * @brief Relay Driver Interface
 *
 * Provides a reusable interface for controlling a relay module
 * connected to a digital GPIO pin. The relay switches an external
 * load (lamp, motor, solenoid) ON or OFF. Supports both active-HIGH
 * and active-LOW relay modules via a configuration flag.
 *
 * Usage:
 *   Relay relay(7, true);   // Pin 7, active-HIGH
 *   relay.init();
 *   relay.turnOn();
 */

#ifndef RELAY_H
#define RELAY_H

#include <Arduino.h>

/**
 * @class Relay
 * @brief Controls a single relay on a specified GPIO pin.
 */
class Relay {
public:
    /**
     * @brief Construct a new Relay object.
     * @param pin        The GPIO pin connected to the relay control input.
     * @param activeHigh True if relay activates on HIGH (default), false for active-LOW.
     */
    Relay(uint8_t pin, bool activeHigh = true);

    /** @brief Initialize the relay pin as OUTPUT and set it to OFF. */
    void init();

    /** @brief Activate the relay (close the switch). */
    void turnOn();

    /** @brief Deactivate the relay (open the switch). */
    void turnOff();

    /**
     * @brief Set relay state from a boolean.
     * @param on True to activate, false to deactivate.
     */
    void setState(bool on);

    /**
     * @brief Check whether the relay is currently activated.
     * @return true if relay is ON, false otherwise.
     */
    bool isOn() const;

private:
    uint8_t _pin;        ///< GPIO pin number
    bool    _activeHigh; ///< True = HIGH activates relay
    bool    _state;      ///< Current relay state (true = ON)
};

#endif // RELAY_H
