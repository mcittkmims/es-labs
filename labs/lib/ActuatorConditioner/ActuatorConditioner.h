/**
 * @file ActuatorConditioner.h
 * @brief Actuator Command Signal Conditioning Module
 *
 * Processes raw actuator commands through a conditioning pipeline:
 *   1. Saturation (clamp to physical limits)
 *   2. Median filter (remove impulsive noise from commands)
 *   3. EWMA smoothing (reduce fluctuations)
 *   4. Ramping (gradual transitions for actuator protection)
 *
 * The first three stages reuse the SignalConditioner library.
 * The ramping stage is added on top to produce smooth transitions.
 *
 * Usage:
 *   ActuatorConditioner cond(5, 0.4, 0.0, 100.0, 5.0);
 *   float output = cond.process(targetDuty);  // call at fixed rate
 */

#ifndef ACTUATOR_CONDITIONER_H
#define ACTUATOR_CONDITIONER_H

#include "SignalConditioner.h"

/**
 * @class ActuatorConditioner
 * @brief Command conditioning pipeline with ramping for analog actuators.
 */
class ActuatorConditioner {
public:
    /**
     * @brief Construct a new ActuatorConditioner.
     * @param medianWindow Median filter window size (odd, <= 9).
     * @param ewmaAlpha EWMA smoothing factor (0.0-1.0).
     * @param minClamp Minimum valid command value.
     * @param maxClamp Maximum valid command value.
     * @param maxRampStep Maximum change per process() call (for ramping).
     */
    ActuatorConditioner(uint8_t medianWindow, float ewmaAlpha,
                        float minClamp, float maxClamp,
                        float maxRampStep);

    /**
     * @brief Process a raw command through the full pipeline.
     * @param rawCommand Raw target value from user input.
     * @return float Conditioned, ramped output value.
     */
    float process(float rawCommand);

    /** @brief Get the current ramped output value. */
    float getRampedOutput() const;

    /** @brief Get the conditioned (pre-ramp) target value. */
    float getConditionedTarget() const;

    /** @brief Access the underlying SignalConditioner for diagnostics. */
    const SignalConditioner& getSignalConditioner() const;

    /** @brief Reset the conditioner and ramp state. */
    void reset();

private:
    SignalConditioner _conditioner;
    float _maxRampStep;
    float _rampedOutput;
    float _conditionedTarget;
    bool  _initialized;
};

#endif // ACTUATOR_CONDITIONER_H
