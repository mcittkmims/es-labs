/**
 * @file PidController.h
 * @brief Reusable discrete PID controller.
 */

#ifndef PID_CONTROLLER_H
#define PID_CONTROLLER_H

enum PidDirection {
    PID_DIRECT = 0,
    PID_REVERSE = 1
};

class PidController {
public:
    PidController(float kp, float ki, float kd,
                  float outputMin, float outputMax,
                  PidDirection direction = PID_DIRECT);

    void init();
    void setTunings(float kp, float ki, float kd);
    void setOutputLimits(float outputMin, float outputMax);
    void setDirection(PidDirection direction);
    void reset();

    float update(float setpoint, float measuredValue, float dtSeconds);

    float getKp() const;
    float getKi() const;
    float getKd() const;
    float getError() const;
    float getIntegral() const;
    float getDerivative() const;
    float getOutput() const;

private:
    float calculateError(float setpoint, float measuredValue) const;
    float clamp(float value, float minValue, float maxValue) const;

    float _kp;
    float _ki;
    float _kd;
    float _outputMin;
    float _outputMax;
    PidDirection _direction;

    bool _hasPreviousError;
    float _previousError;
    float _integral;
    float _derivative;
    float _lastError;
    float _lastOutput;
};

#endif // PID_CONTROLLER_H
