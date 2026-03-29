/**
 * @file PwmActuator.h
 * @brief PWM Analog Actuator Driver Interface
 *
 * Provides a reusable interface for controlling an analog actuator
 * (DC motor, dimmable LED, fan) via a PWM-capable GPIO pin. Accepts
 * a duty cycle percentage (0--100%) and converts it to the 0--255
 * range used by Arduino's analogWrite().
 *
 * Usage:
 *   PwmActuator motor(9);
 *   motor.init();
 *   motor.setDutyCycle(75.0f);  // 75% power
 */

#ifndef PWM_ACTUATOR_H
#define PWM_ACTUATOR_H

#include <Arduino.h>

/**
 * @class PwmActuator
 * @brief Controls an analog actuator via PWM output.
 */
class PwmActuator {
public:
    /**
     * @brief Construct a new PwmActuator object.
     * @param pin PWM-capable GPIO pin number.
     */
    PwmActuator(uint8_t pin);

    /** @brief Initialize the PWM pin as OUTPUT and set duty to 0%. */
    void init();

    /**
     * @brief Set the PWM duty cycle.
     * @param percent Duty cycle as a percentage (0.0--100.0).
     *                Values are clamped to the valid range.
     */
    void setDutyCycle(float percent);

    /**
     * @brief Get the current duty cycle percentage.
     * @return float Current duty cycle (0.0--100.0).
     */
    float getDutyCycle() const;

    /**
     * @brief Get the current raw PWM value (0--255).
     * @return uint8_t Raw analogWrite value.
     */
    uint8_t getRawPwm() const;

private:
    uint8_t _pin;       ///< PWM GPIO pin number
    float   _dutyCycle; ///< Current duty cycle percentage
};

#endif // PWM_ACTUATOR_H
