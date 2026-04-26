/**
 * @file PidController.cpp
 * @brief Reusable discrete PID controller implementation.
 */

#include "PidController.h"

PidController::PidController(float kp, float ki, float kd,
                             float outputMin, float outputMax,
                             PidDirection direction)
    : _kp(kp),
      _ki(ki),
      _kd(kd),
      _outputMin(outputMin),
      _outputMax(outputMax),
      _direction(direction),
      _hasPreviousError(false),
      _previousError(0.0f),
      _integral(0.0f),
      _derivative(0.0f),
      _lastError(0.0f),
      _lastOutput(0.0f) {}

void PidController::init() {
    reset();
}

void PidController::setTunings(float kp, float ki, float kd) {
    if (kp < 0.0f) kp = 0.0f;
    if (ki < 0.0f) ki = 0.0f;
    if (kd < 0.0f) kd = 0.0f;
    _kp = kp;
    _ki = ki;
    _kd = kd;
}

void PidController::setOutputLimits(float outputMin, float outputMax) {
    if (outputMax < outputMin) {
        float tmp = outputMax;
        outputMax = outputMin;
        outputMin = tmp;
    }

    _outputMin = outputMin;
    _outputMax = outputMax;
    _integral = clamp(_integral, _outputMin, _outputMax);
    _lastOutput = clamp(_lastOutput, _outputMin, _outputMax);
}

void PidController::setDirection(PidDirection direction) {
    _direction = direction;
}

void PidController::reset() {
    _hasPreviousError = false;
    _previousError = 0.0f;
    _integral = 0.0f;
    _derivative = 0.0f;
    _lastError = 0.0f;
    _lastOutput = 0.0f;
}

float PidController::update(float setpoint, float measuredValue, float dtSeconds) {
    if (dtSeconds <= 0.0f) {
        dtSeconds = 0.001f;
    }

    float error = calculateError(setpoint, measuredValue);
    float proportional = _kp * error;

    _integral += _ki * error * dtSeconds;
    _integral = clamp(_integral, _outputMin, _outputMax);

    if (_hasPreviousError) {
        _derivative = (error - _previousError) / dtSeconds;
    } else {
        _derivative = 0.0f;
        _hasPreviousError = true;
    }

    float derivativeTerm = _kd * _derivative;
    float output = proportional + _integral + derivativeTerm;
    output = clamp(output, _outputMin, _outputMax);

    _previousError = error;
    _lastError = error;
    _lastOutput = output;
    return output;
}

float PidController::getKp() const {
    return _kp;
}

float PidController::getKi() const {
    return _ki;
}

float PidController::getKd() const {
    return _kd;
}

float PidController::getError() const {
    return _lastError;
}

float PidController::getIntegral() const {
    return _integral;
}

float PidController::getDerivative() const {
    return _derivative;
}

float PidController::getOutput() const {
    return _lastOutput;
}

float PidController::calculateError(float setpoint, float measuredValue) const {
    if (_direction == PID_REVERSE) {
        return measuredValue - setpoint;
    }
    return setpoint - measuredValue;
}

float PidController::clamp(float value, float minValue, float maxValue) const {
    if (value < minValue) return minValue;
    if (value > maxValue) return maxValue;
    return value;
}
