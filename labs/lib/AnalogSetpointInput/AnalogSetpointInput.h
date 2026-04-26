/**
 * @file AnalogSetpointInput.h
 * @brief Potentiometer-based setpoint input wrapper.
 *
 * Encapsulates ADC reads and maps the raw value to a configurable
 * engineering range, such as a temperature setpoint in degrees Celsius.
 */

#ifndef ANALOG_SETPOINT_INPUT_H
#define ANALOG_SETPOINT_INPUT_H

#include <Arduino.h>

class AnalogSetpointInput {
public:
    /**
     * @brief Construct a setpoint input.
     * @param adcPin Analog pin connected to the potentiometer wiper.
     * @param minValue Engineering value at ADC 0.
     * @param maxValue Engineering value at ADC max.
     * @param adcResolution ADC resolution in bits, 10 for Arduino Mega.
     */
    AnalogSetpointInput(uint8_t adcPin, float minValue, float maxValue,
                        uint8_t adcResolution = 10);

    /** @brief Configure the ADC pin. */
    void init();

    /** @brief Read raw ADC value. */
    uint16_t readRaw();

    /** @brief Read and map the current value to the configured range. */
    float readValue();

    /** @brief Update the mapped engineering range. */
    void setRange(float minValue, float maxValue);

    /** @brief Return the last raw ADC sample. */
    uint16_t getLastRaw() const;

    /** @brief Return the last mapped setpoint value. */
    float getLastValue() const;

private:
    uint8_t _adcPin;
    float _minValue;
    float _maxValue;
    uint16_t _adcMax;
    uint16_t _lastRaw;
    float _lastValue;
};

#endif // ANALOG_SETPOINT_INPUT_H
