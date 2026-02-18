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
     * @brief Check whether the LED is currently ON.
     * @return true if the LED is ON, false otherwise.
     */
    bool isOn() const;

private:
    uint8_t ledPin;  ///< GPIO pin number
    bool state;      ///< Current LED state (true = ON, false = OFF)
};

#endif // LED_H
