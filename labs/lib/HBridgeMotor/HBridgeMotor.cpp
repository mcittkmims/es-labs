/**
 * @file HBridgeMotor.cpp
 * @brief H-bridge DC motor driver implementation.
 */

#include "HBridgeMotor.h"

HBridgeMotor::HBridgeMotor(uint8_t enablePwmPin, uint8_t input1Pin, uint8_t input2Pin)
    : _enable(enablePwmPin),
      _input1Pin(input1Pin),
      _input2Pin(input2Pin),
      _direction(HBRIDGE_STOPPED) {}

void HBridgeMotor::init() {
    pinMode(_input1Pin, OUTPUT);
    pinMode(_input2Pin, OUTPUT);
    _enable.init();
    stop();
}

void HBridgeMotor::setForward(float dutyPercent) {
    setDirectionPins(HBRIDGE_FORWARD);
    _enable.setDuty(dutyPercent);
}

void HBridgeMotor::setReverse(float dutyPercent) {
    setDirectionPins(HBRIDGE_REVERSE);
    _enable.setDuty(dutyPercent);
}

void HBridgeMotor::stop() {
    setDirectionPins(HBRIDGE_STOPPED);
    _enable.setDuty(0.0f);
}

float HBridgeMotor::getDuty() const {
    return _enable.getDuty();
}

uint8_t HBridgeMotor::getRawPwm() const {
    return _enable.getRawPwm();
}

HBridgeDirection HBridgeMotor::getDirection() const {
    return _direction;
}

void HBridgeMotor::setDirectionPins(HBridgeDirection direction) {
    _direction = direction;

    switch (direction) {
        case HBRIDGE_FORWARD:
            digitalWrite(_input1Pin, HIGH);
            digitalWrite(_input2Pin, LOW);
            break;
        case HBRIDGE_REVERSE:
            digitalWrite(_input1Pin, LOW);
            digitalWrite(_input2Pin, HIGH);
            break;
        case HBRIDGE_STOPPED:
        default:
            digitalWrite(_input1Pin, LOW);
            digitalWrite(_input2Pin, LOW);
            break;
    }
}
