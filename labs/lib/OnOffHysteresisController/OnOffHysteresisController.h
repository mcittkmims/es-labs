/**
 * @file OnOffHysteresisController.h
 * @brief Heating-style ON-OFF controller with hysteresis.
 *
 * The controller turns the actuator ON when the measured value falls below
 * the lower threshold and turns it OFF when the measured value rises above
 * the upper threshold. Values inside the hysteresis band preserve the last
 * output, preventing relay chatter around the setpoint.
 */

#ifndef ON_OFF_HYSTERESIS_CONTROLLER_H
#define ON_OFF_HYSTERESIS_CONTROLLER_H

#include <Arduino.h>

enum HysteresisOutputState {
    HYSTERESIS_OUTPUT_OFF = 0,
    HYSTERESIS_OUTPUT_ON = 1
};

class OnOffHysteresisController {
public:
    /**
     * @brief Construct the controller.
     * @param setpoint Desired measured value.
     * @param hysteresisBand Full hysteresis band around the setpoint.
     */
    OnOffHysteresisController(float setpoint, float hysteresisBand);

    /** @brief Reset output to OFF. */
    void init();

    /** @brief Update controller configuration at runtime. */
    void setConfig(float setpoint, float hysteresisBand);

    /**
     * @brief Process one measured sample.
     * @return true when actuator command is ON.
     */
    bool update(float measuredValue);

    /** @brief Force the output to a known state. */
    void forceOutput(bool outputOn);

    /** @brief Current actuator command. */
    bool isOutputOn() const;

    /** @brief Current output state enum. */
    HysteresisOutputState getState() const;

    /** @brief Lower threshold where a heating actuator turns ON. */
    float getLowerThreshold() const;

    /** @brief Upper threshold where a heating actuator turns OFF. */
    float getUpperThreshold() const;

    /** @brief Configured setpoint. */
    float getSetpoint() const;

    /** @brief Configured full hysteresis band. */
    float getHysteresisBand() const;

private:
    float _setpoint;
    float _hysteresisBand;
    HysteresisOutputState _state;
};

#endif // ON_OFF_HYSTERESIS_CONTROLLER_H
