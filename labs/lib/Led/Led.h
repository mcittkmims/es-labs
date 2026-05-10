/**
 * @file Led.h
 * @brief LED Driver Interface
 *
 * Provides a simple, reusable interface for controlling an LED
 * connected to a digital GPIO pin. Supports initialization,
 * turning ON/OFF, toggling, and querying the current state.
 *
 * Usage:
 *   Led myLed(pinNumber);
 *   myLed.init();
 *   myLed.turnOn();
 */

#ifndef LED_H
#define LED_H

#include <Arduino.h>

/**
 * @class Led
 * @brief Controls a single LED on a specified GPIO pin.
 */
class Led {
public:
    /**
     * @brief Construct a new Led object.
     * @param pin The GPIO pin number where the LED is connected.
     */
    Led(uint8_t pin);

    /**
     * @brief Initialize the LED pin as OUTPUT and set it to OFF.
     */
    void init();

    /**
     * @brief Turn the LED ON (set pin HIGH).
     */
    void turnOn();

    /**
     * @brief Turn the LED OFF (set pin LOW).
     */
    void turnOff();

    /**
     * @brief Toggle the LED state (ON becomes OFF, OFF becomes ON).
     */
    void toggle();

    /**
     * @brief Drive the LED to a specific logical state.
     *
     * Convenience method that maps a boolean input directly onto
     * @ref turnOn() / @ref turnOff(), useful when applying the output
     * of a finite-state machine or any boolean controller.
     *
     * @param on true to turn the LED ON, false to turn it OFF.
     */
    void set(bool on);

    /**
     * @brief Check whether the LED is currently ON.
     * @return true if the LED is ON, false otherwise.
     */
    bool isOn() const;

private:
    uint8_t ledPin;  ///< GPIO pin number
    bool state;      ///< Current LED state (true = ON, false = OFF)
};

#endif // LED_H
