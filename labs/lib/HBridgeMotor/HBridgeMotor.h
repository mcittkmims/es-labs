/**
 * @file HBridgeMotor.h
 * @brief Unidirectional/bidirectional DC motor driver for an H-bridge.
 */

#ifndef HBRIDGE_MOTOR_H
#define HBRIDGE_MOTOR_H

#include <Arduino.h>
#include "PwmActuator.h"

enum HBridgeDirection {
    HBRIDGE_STOPPED = 0,
    HBRIDGE_FORWARD = 1,
    HBRIDGE_REVERSE = 2
};

class HBridgeMotor {
public:
    HBridgeMotor(uint8_t enablePwmPin, uint8_t input1Pin, uint8_t input2Pin);

    void init();
    void setForward(float dutyPercent);
    void setReverse(float dutyPercent);
    void stop();

    float getDuty() const;
    uint8_t getRawPwm() const;
    HBridgeDirection getDirection() const;

private:
    void setDirectionPins(HBridgeDirection direction);

    PwmActuator _enable;
    uint8_t _input1Pin;
    uint8_t _input2Pin;
    HBridgeDirection _direction;
};

#endif // HBRIDGE_MOTOR_H
