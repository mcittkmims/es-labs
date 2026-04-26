/**
 * @file DhtSensor.cpp
 * @brief DHT temperature and humidity sensor wrapper implementation.
 */

#include "DhtSensor.h"
#include <math.h>

DhtSensor::DhtSensor(uint8_t dataPin, uint8_t sensorType)
    : _dht(dataPin, sensorType),
      _temperatureC(NAN),
      _humidityPercent(NAN),
      _valid(false) {}

void DhtSensor::init() {
    _dht.begin();
    _temperatureC = NAN;
    _humidityPercent = NAN;
    _valid = false;
}

bool DhtSensor::read() {
    float humidity = _dht.readHumidity();
    float temperature = _dht.readTemperature();

    if (isnan(humidity) || isnan(temperature)) {
        _valid = false;
        return false;
    }

    _humidityPercent = humidity;
    _temperatureC = temperature;
    _valid = true;
    return true;
}

float DhtSensor::getTemperatureC() const {
    return _temperatureC;
}

float DhtSensor::getHumidityPercent() const {
    return _humidityPercent;
}

bool DhtSensor::isValid() const {
    return _valid;
}
