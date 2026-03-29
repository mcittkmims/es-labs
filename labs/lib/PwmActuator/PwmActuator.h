/**
 * @file PwmActuator.h
 * @brief PWM-Based Analog Actuator Driver Interface
 *
 * Provides a reusable abstraction for controlling an analog actuator
 * via PWM output. Accepts a duty cycle percentage (0-100%) and maps
 * it to 0-255 for analogWrite().
 *
 * Usage:
 *   PwmActuator motor(6);  // PWM pin 6
 *   motor.init();
 *   motor.setDuty(75.0);   // 75% duty cycle
 */

#ifndef PWM_ACTUATOR_H
#define PWM_ACTUATOR_H

#include <Arduino.h>

/**
 * @class PwmActuator
 * @brief Controls an analog actuator via PWM duty cycle.
 */
class PwmActuator {
public:
    /**
     * @brief Construct a new PwmActuator object.
     * @param pin PWM-capable GPIO pin.
     */
    PwmActuator(uint8_t pin);

    /** @brief Initialize the PWM pin as OUTPUT, set duty to 0. */
    void init();

    /**
     * @brief Set the duty cycle (0.0 to 100.0%).
     * @param duty Duty cycle percentage. Clamped to [0, 100].
     */
    void setDuty(float duty);

    /** @brief Get the current duty cycle percentage. */
    float getDuty() const;

    /** @brief Get the current raw PWM value (0-255). */
    uint8_t getRawPwm() const;

private:
    uint8_t _pin;
    float   _duty;
};

#endif // PWM_ACTUATOR_H
