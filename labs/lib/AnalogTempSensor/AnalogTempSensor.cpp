/**
 * @file AnalogTempSensor.cpp
 * @brief NTC Thermistor Analog Temperature Sensor Driver Implementation
 *
 * Implements the ADC-based temperature reading for an NTC thermistor
 * connected through a voltage divider circuit. Uses the Steinhart-Hart
 * Beta parameter equation for resistance-to-temperature conversion.
 *
 * Conversion formula (Beta equation):
 *   R_ntc = R_series * ADC / (ADC_MAX - ADC)
 *   1/T   = 1/T0 + (1/B) * ln(R_ntc / R0)
 *   T_C   = T_K - 273.15
 */

#include "AnalogTempSensor.h"
#include <math.h>

// ──────────────────────────────────────────────────────────────────────────
// Constructor
// ──────────────────────────────────────────────────────────────────────────

AnalogTempSensor::AnalogTempSensor(uint8_t adcPin, uint32_t seriesR,
                                   uint32_t nominalR, uint16_t betaCoeff,
                                   float nominalTempC, uint16_t adcResolution)
    : _adcPin(adcPin),
      _seriesR(seriesR),
      _nominalR(nominalR),
      _betaCoeff(betaCoeff),
      _nominalTempK(nominalTempC + 273.15f),
      _adcMax((1 << adcResolution) - 1),
      _lastRaw(0),
      _lastResistance(-1.0f),
      _lastTempC(NAN),
      _valid(false) {}

// ──────────────────────────────────────────────────────────────────────────
// Initialization
// ──────────────────────────────────────────────────────────────────────────

void AnalogTempSensor::init() {
    // On AVR, analog pins are automatically configured when analogRead()
    // is called, but we set the pin mode explicitly for clarity.
    pinMode(_adcPin, INPUT);
    _lastRaw        = 0;
    _lastResistance = -1.0f;
    _lastTempC      = NAN;
    _valid          = false;
}

// ──────────────────────────────────────────────────────────────────────────
// Raw ADC reading
// ──────────────────────────────────────────────────────────────────────────

uint16_t AnalogTempSensor::readRaw() {
    _lastRaw = analogRead(_adcPin);
    return _lastRaw;
}

// ──────────────────────────────────────────────────────────────────────────
// Resistance calculation from ADC
// ──────────────────────────────────────────────────────────────────────────

float AnalogTempSensor::readResistance() {
    uint16_t adc = readRaw();

    // Guard against division by zero or open/short circuit.
    // ADC = 0 means NTC is shorted (R → 0).
    // ADC = ADC_MAX means NTC is open circuit (R → ∞).
    if (adc == 0 || adc >= _adcMax) {
        _lastResistance = -1.0f;
        _valid = false;
        return -1.0f;
    }

    // Voltage divider: VCC ─ R_series ─ ADC_PIN ─ NTC ─ GND
    // V_adc / VCC = R_ntc / (R_series + R_ntc)
    // => R_ntc = R_series * ADC / (ADC_MAX - ADC)
    float resistance = (float)_seriesR * (float)adc / (float)(_adcMax - adc);
    _lastResistance = resistance;
    _valid = true;
    return resistance;
}

// ──────────────────────────────────────────────────────────────────────────
// Temperature conversion (Beta equation)
// ──────────────────────────────────────────────────────────────────────────

float AnalogTempSensor::readTemperatureC() {
    float resistance = readResistance();

    if (resistance < 0.0f) {
        _lastTempC = NAN;
        _valid = false;
        return NAN;
    }

    // Steinhart-Hart simplified (Beta parameter equation):
    // 1/T = 1/T0 + (1/B) * ln(R / R0)
    float steinhart = log(resistance / (float)_nominalR);  // ln(R / R0)
    steinhart /= (float)_betaCoeff;                         // (1/B) * ln(R/R0)
    steinhart += 1.0f / _nominalTempK;                      // + 1/T0

    // Convert from Kelvin to Celsius
    float tempK = 1.0f / steinhart;
    _lastTempC = tempK - 273.15f;
    _valid = true;

    return _lastTempC;
}

// ──────────────────────────────────────────────────────────────────────────
// Temperature in Fahrenheit
// ──────────────────────────────────────────────────────────────────────────

float AnalogTempSensor::readTemperatureF() {
    float tempC = readTemperatureC();
    if (isnan(tempC)) return NAN;
    return tempC * 9.0f / 5.0f + 32.0f;
}

// ──────────────────────────────────────────────────────────────────────────
// Accessors
// ──────────────────────────────────────────────────────────────────────────

bool AnalogTempSensor::isValid() const {
    return _valid;
}

uint16_t AnalogTempSensor::getLastRaw() const {
    return _lastRaw;
}

float AnalogTempSensor::getLastTemperatureC() const {
    return _lastTempC;
}

float AnalogTempSensor::getLastResistance() const {
    return _lastResistance;
}
