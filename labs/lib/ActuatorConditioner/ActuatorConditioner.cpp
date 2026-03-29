/**
 * @file ActuatorConditioner.cpp
 * @brief Actuator Command Signal Conditioning Implementation
 */

#include "ActuatorConditioner.h"
#include <math.h>

ActuatorConditioner::ActuatorConditioner(uint8_t medianWindow, float ewmaAlpha,
                                         float minClamp, float maxClamp,
                                         float maxRampStep)
    : _conditioner(medianWindow, ewmaAlpha, minClamp, maxClamp),
      _maxRampStep(maxRampStep),
      _rampedOutput(0.0f),
      _conditionedTarget(0.0f),
      _initialized(false) {}

float ActuatorConditioner::process(float rawCommand) {
    // Stages 1-3: saturation, median, EWMA
    _conditionedTarget = _conditioner.process(rawCommand);

    // Stage 4: ramping (gradual transition)
    if (!_initialized) {
        _rampedOutput = _conditionedTarget;
        _initialized = true;
    } else {
        float diff = _conditionedTarget - _rampedOutput;
        if (diff > _maxRampStep) {
            _rampedOutput += _maxRampStep;
        } else if (diff < -_maxRampStep) {
            _rampedOutput -= _maxRampStep;
        } else {
            _rampedOutput = _conditionedTarget;
        }
    }

    return _rampedOutput;
}

float ActuatorConditioner::getRampedOutput() const {
    return _rampedOutput;
}

float ActuatorConditioner::getConditionedTarget() const {
    return _conditionedTarget;
}

const SignalConditioner& ActuatorConditioner::getSignalConditioner() const {
    return _conditioner;
}

void ActuatorConditioner::reset() {
    _conditioner.reset();
    _rampedOutput = 0.0f;
    _conditionedTarget = 0.0f;
    _initialized = false;
}
