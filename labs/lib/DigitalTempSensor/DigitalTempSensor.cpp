/**
 * @file DigitalTempSensor.cpp
 * @brief DS18B20 Digital Temperature Sensor Driver Implementation
 *
 * Implements OneWire-based temperature reading from the DS18B20 sensor.
 * Uses the DallasTemperature library for protocol handling and provides
 * both blocking and non-blocking read modes.
 */

#include "DigitalTempSensor.h"

/** DS18B20 returns -127.0°C when disconnected. */
static const float DISCONNECTED_TEMP = -127.0f;

/** DS18B20 returns +85.0°C on power-on reset (no conversion started). */
static const float POWER_ON_RESET_TEMP = 85.0f;

// ──────────────────────────────────────────────────────────────────────────
// Constructor
// ──────────────────────────────────────────────────────────────────────────

DigitalTempSensor::DigitalTempSensor(uint8_t dataPin, uint8_t resolution)
    : _oneWire(dataPin),
      _sensors(&_oneWire),
      _resolution(resolution),
      _lastTempC(NAN),
      _connected(false),
      _valid(false),
      _deviceCount(0) {}

// ──────────────────────────────────────────────────────────────────────────
// Initialization
// ──────────────────────────────────────────────────────────────────────────

bool DigitalTempSensor::init() {
    _sensors.begin();
    _deviceCount = _sensors.getDeviceCount();

    if (_deviceCount == 0) {
        _connected = false;
        return false;
    }

    _connected = true;

    // Set the measurement resolution for all devices on the bus.
    // Resolution affects both accuracy and conversion time:
    //   9-bit:  0.5°C   precision, ~94 ms conversion
    //   10-bit: 0.25°C  precision, ~188 ms conversion
    //   11-bit: 0.125°C precision, ~375 ms conversion
    //   12-bit: 0.0625°C precision, ~750 ms conversion
    _sensors.setResolution(_resolution);

    // Use non-blocking mode so we can check conversion status.
    _sensors.setWaitForConversion(false);

    return true;
}

// ──────────────────────────────────────────────────────────────────────────
// Conversion control
// ──────────────────────────────────────────────────────────────────────────

void DigitalTempSensor::requestConversion() {
    if (_connected) {
        _sensors.requestTemperatures();
    }
}

bool DigitalTempSensor::isConversionComplete() {
    if (!_connected) return false;
    return _sensors.isConversionComplete();
}

// ──────────────────────────────────────────────────────────────────────────
// Temperature reading (blocking)
// ──────────────────────────────────────────────────────────────────────────

float DigitalTempSensor::readTemperatureC() {
    if (!_connected) {
        _valid = false;
        _lastTempC = NAN;
        return NAN;
    }

    // Request conversion and wait for completion.
    _sensors.setWaitForConversion(true);
    _sensors.requestTemperatures();
    _sensors.setWaitForConversion(false);

    float tempC = _sensors.getTempCByIndex(0);

    // Validate the reading — DS18B20 returns specific values on error.
    if (tempC == DEVICE_DISCONNECTED_C || tempC <= DISCONNECTED_TEMP) {
        _valid = false;
        _lastTempC = NAN;
        return NAN;
    }

    // Ignore power-on reset value (85.0°C) — indicates no valid conversion.
    if (tempC == POWER_ON_RESET_TEMP) {
        _valid = false;
        return _lastTempC;  // Return last known-good value.
    }

    _valid = true;
    _lastTempC = tempC;
    return _lastTempC;
}

// ──────────────────────────────────────────────────────────────────────────
// Accessors
// ──────────────────────────────────────────────────────────────────────────

float DigitalTempSensor::getLastTemperatureC() const {
    return _lastTempC;
}

float DigitalTempSensor::readTemperatureF() {
    float tempC = readTemperatureC();
    if (isnan(tempC)) return NAN;
    return tempC * 9.0f / 5.0f + 32.0f;
}

bool DigitalTempSensor::isConnected() const {
    return _connected;
}

bool DigitalTempSensor::isValid() const {
    return _valid;
}

uint8_t DigitalTempSensor::getDeviceCount() const {
    return _deviceCount;
}
