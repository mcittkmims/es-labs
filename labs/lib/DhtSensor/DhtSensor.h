/**
 * @file DhtSensor.h
 * @brief DHT temperature and humidity sensor wrapper.
 *
 * Provides a small reusable interface around the Arduino DHT library.
 * The wrapper keeps the last valid temperature and humidity values so
 * application code can distinguish sensor failures from valid samples.
 */

#ifndef DHT_SENSOR_H
#define DHT_SENSOR_H

#include <Arduino.h>
#include <DHT.h>

class DhtSensor {
public:
    /**
     * @brief Construct a DHT sensor wrapper.
     * @param dataPin GPIO pin connected to the DHT data line.
     * @param sensorType DHT11, DHT21, or DHT22 constant from DHT.h.
     */
    DhtSensor(uint8_t dataPin, uint8_t sensorType);

    /** @brief Initialize the DHT driver. */
    void init();

    /**
     * @brief Read both temperature and humidity.
     * @return true when both values are valid; false on checksum/timeout error.
     */
    bool read();

    /** @brief Get the last valid temperature in degrees Celsius. */
    float getTemperatureC() const;

    /** @brief Get the last valid relative humidity percentage. */
    float getHumidityPercent() const;

    /** @brief True if the last read produced valid values. */
    bool isValid() const;

private:
    DHT _dht;
    float _temperatureC;
    float _humidityPercent;
    bool _valid;
};

#endif // DHT_SENSOR_H
