/**
 * @file DigitalTempSensor.h
 * @brief DS18B20 Digital Temperature Sensor Driver Interface (OneWire)
 *
 * Provides a reusable abstraction for reading temperature from a Dallas
 * DS18B20 digital temperature sensor using the OneWire protocol. The
 * DS18B20 communicates over a single data wire and provides 9-to-12-bit
 * temperature readings in degrees Celsius.
 *
 * Circuit configuration:
 *   VCC (5V) ──┬── DS18B20 VDD
 *              │
 *           [4.7K]  (pull-up resistor)
 *              │
 *   DATA_PIN ──┼── DS18B20 DQ
 *              │
 *   GND ───────┴── DS18B20 GND
 *
 * Usage:
 *   DigitalTempSensor ds(2);  // OneWire data on pin 2
 *   ds.init();
 *   float tempC = ds.readTemperatureC();
 */

#ifndef DIGITAL_TEMP_SENSOR_H
#define DIGITAL_TEMP_SENSOR_H

#include <Arduino.h>
#include <OneWire.h>
#include <DallasTemperature.h>

/**
 * @class DigitalTempSensor
 * @brief Reads temperature from a DS18B20 sensor via OneWire protocol.
 *
 * Wraps the OneWire and DallasTemperature libraries behind a clean
 * interface consistent with the AnalogTempSensor API.
 */
class DigitalTempSensor {
public:
    /**
     * @brief Construct a new DigitalTempSensor object.
     *
     * @param dataPin Digital pin connected to the DS18B20 DQ line.
     * @param resolution Sensor resolution in bits (9–12, default: 10).
     *                   Higher resolution = more accurate but slower conversion.
     *                   9-bit: 93.75 ms, 10-bit: 187.5 ms,
     *                   11-bit: 375 ms, 12-bit: 750 ms.
     */
    DigitalTempSensor(uint8_t dataPin, uint8_t resolution = 10);

    /**
     * @brief Initialize the sensor and OneWire bus.
     *
     * Scans the bus, sets the resolution, and optionally waits for
     * the first conversion to complete.
     *
     * @return true if at least one DS18B20 device was found on the bus.
     */
    bool init();

    /**
     * @brief Request a new temperature conversion from the sensor.
     *
     * This is a non-blocking call that starts the conversion process.
     * Use isConversionComplete() to check when the reading is ready,
     * then call readTemperatureC() to retrieve the result.
     */
    void requestConversion();

    /**
     * @brief Check if the temperature conversion is complete.
     * @return true if the conversion is done and a reading is available.
     */
    bool isConversionComplete();

    /**
     * @brief Read temperature in degrees Celsius (blocking).
     *
     * Requests a conversion and waits for it to complete before
     * returning the result.
     *
     * @return float Temperature in °C. Returns DEVICE_DISCONNECTED_C
     *               (-127.0) if the sensor is not responding.
     */
    float readTemperatureC();

    /**
     * @brief Read the result of the last completed non-blocking conversion.
     *
     * Call this after isConversionComplete() returns true. Does NOT trigger
     * a new conversion. Validates the result and updates _valid / _lastTempC.
     *
     * @return float Temperature in °C. Returns NAN if reading is invalid.
     */
    float readLastConversionC();

    /**
     * @brief Get the cached temperature from the previous valid read.
     * @return float Last temperature in °C (NAN if no valid read yet).
     */
    float getLastTemperatureC() const;

    /**
     * @brief Read temperature in degrees Fahrenheit (blocking).
     * @return float Temperature in °F.
     */
    float readTemperatureF();

    /**
     * @brief Check if the sensor was found during initialization.
     * @return true if a DS18B20 device is present on the bus.
     */
    bool isConnected() const;

    /**
     * @brief Check if the last reading was valid.
     *
     * A reading is invalid if the sensor returns -127.0°C (disconnected)
     * or +85.0°C (power-on reset / conversion not started).
     *
     * @return true if the last reading was valid.
     */
    bool isValid() const;

    /**
     * @brief Get the number of DS18B20 devices found on the bus.
     * @return uint8_t Number of devices.
     */
    uint8_t getDeviceCount() const;

private:
    OneWire           _oneWire;       /**< OneWire bus instance.           */
    DallasTemperature _sensors;       /**< DallasTemperature library instance. */
    uint8_t           _resolution;    /**< Configured resolution (9–12 bits). */
    float             _lastTempC;     /**< Last valid temperature reading.    */
    bool              _connected;     /**< True if sensor found on bus.       */
    bool              _valid;         /**< True if last reading was valid.     */
    uint8_t           _deviceCount;   /**< Number of devices on the bus.      */
};

#endif // DIGITAL_TEMP_SENSOR_H
