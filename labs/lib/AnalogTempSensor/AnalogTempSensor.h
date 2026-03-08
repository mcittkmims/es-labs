/**
 * @file AnalogTempSensor.h
 * @brief NTC Thermistor Analog Temperature Sensor Driver Interface
 *
 * Provides a reusable abstraction for reading temperature from an NTC
 * thermistor connected to an analog ADC pin through a voltage divider
 * circuit. Converts raw ADC readings to temperature in degrees Celsius
 * using the Steinhart-Hart Beta parameter equation.
 *
 * Circuit configuration (voltage divider):
 *   VCC ─── [R_series] ─── ADC_PIN ─── [NTC] ─── GND
 *
 * The NTC resistance is calculated from the ADC reading and then
 * converted to temperature using:
 *   1/T = 1/T0 + (1/B) * ln(R_ntc / R0)
 *
 * Usage:
 *   AnalogTempSensor ntc(A0, 10000, 10000, 3950);
 *   ntc.init();
 *   float tempC = ntc.readTemperatureC();
 */

#ifndef ANALOG_TEMP_SENSOR_H
#define ANALOG_TEMP_SENSOR_H

#include <Arduino.h>

/**
 * @class AnalogTempSensor
 * @brief Reads temperature from an NTC thermistor via ADC.
 *
 * Encapsulates the ADC reading and Steinhart-Hart conversion for
 * an NTC thermistor in a voltage divider configuration.
 */
class AnalogTempSensor {
public:
    /**
     * @brief Construct a new AnalogTempSensor object.
     *
     * @param adcPin      Analog input pin connected to the voltage divider midpoint.
     * @param seriesR     Series resistor value in ohms (typically 10000).
     * @param nominalR    NTC nominal resistance at reference temperature (typically 10000).
     * @param betaCoeff   Beta coefficient of the NTC thermistor (typically 3950).
     * @param nominalTempC Nominal temperature for R0 in Celsius (default 25°C).
     * @param adcResolution ADC resolution in bits (default 10 for Arduino).
     */
    AnalogTempSensor(uint8_t adcPin, uint32_t seriesR, uint32_t nominalR,
                     uint16_t betaCoeff, float nominalTempC = 25.0f,
                     uint16_t adcResolution = 10);

    /**
     * @brief Initialize the sensor (configures the ADC pin as input).
     */
    void init();

    /**
     * @brief Read the raw ADC value from the sensor pin.
     * @return uint16_t Raw ADC value (0 to 2^adcResolution - 1).
     */
    uint16_t readRaw();

    /**
     * @brief Read the NTC resistance calculated from the ADC reading.
     * @return float Resistance in ohms. Returns -1.0 if reading is invalid.
     */
    float readResistance();

    /**
     * @brief Read temperature in degrees Celsius.
     *
     * Performs ADC reading, calculates NTC resistance, and converts
     * to temperature using the Beta parameter equation.
     *
     * @return float Temperature in °C. Returns NAN if reading is invalid.
     */
    float readTemperatureC();

    /**
     * @brief Read temperature in degrees Fahrenheit.
     * @return float Temperature in °F. Returns NAN if reading is invalid.
     */
    float readTemperatureF();

    /**
     * @brief Check if the last reading was valid.
     *
     * A reading is invalid if the ADC returns 0 or max value,
     * indicating a short circuit or open circuit condition.
     *
     * @return true if the last reading was valid.
     */
    bool isValid() const;

    /**
     * @brief Get the last raw ADC value without performing a new reading.
     * @return uint16_t Last raw ADC value.
     */
    uint16_t getLastRaw() const;

    /**
     * @brief Get the last computed temperature without performing a new reading.
     * @return float Last temperature in °C.
     */
    float getLastTemperatureC() const;

    /**
     * @brief Get the last computed resistance without performing a new reading.
     * @return float Last NTC resistance in ohms, or -1.0 if last read was invalid.
     */
    float getLastResistance() const;

private:
    uint8_t  _adcPin;          /**< Analog input pin number.                */
    uint32_t _seriesR;         /**< Series resistor value (ohms).           */
    uint32_t _nominalR;        /**< NTC nominal resistance at T0 (ohms).    */
    uint16_t _betaCoeff;       /**< Beta coefficient of the NTC.            */
    float    _nominalTempK;    /**< Nominal temperature in Kelvin.          */
    uint16_t _adcMax;          /**< Maximum ADC value (2^resolution - 1).   */
    uint16_t _lastRaw;         /**< Last raw ADC reading.                   */
    float    _lastResistance;  /**< Last computed NTC resistance (ohms).    */
    float    _lastTempC;       /**< Last computed temperature (°C).         */
    bool     _valid;           /**< Validity flag for last reading.         */
};

#endif // ANALOG_TEMP_SENSOR_H
