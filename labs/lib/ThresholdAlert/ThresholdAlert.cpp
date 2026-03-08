/**
 * @file ThresholdAlert.cpp
 * @brief Hysteresis-Based Threshold Alert Module Implementation
 *
 * Implements the 4-state FSM for threshold detection with hysteresis
 * and debounce filtering. The state machine transitions:
 *
 *   NORMAL ──> DEBOUNCE_HIGH ──> ALERT_ACTIVE ──> DEBOUNCE_LOW ──> NORMAL
 *
 * Each transition requires a configurable number of consecutive readings
 * confirming the new condition, preventing transient spikes from causing
 * false state changes.
 */

#include "ThresholdAlert.h"

// ──────────────────────────────────────────────────────────────────────────
// State name lookup table
// ──────────────────────────────────────────────────────────────────────────

static const char* STATE_NAMES[] = {
    "NORMAL",
    "DEBOUNCE_HIGH",
    "ALERT",
    "DEBOUNCE_LOW"
};

// ──────────────────────────────────────────────────────────────────────────
// Constructor
// ──────────────────────────────────────────────────────────────────────────

ThresholdAlert::ThresholdAlert(float highThreshold, float lowThreshold,
                               uint8_t debounceCount)
    : _highThreshold(highThreshold),
      _lowThreshold(lowThreshold),
      _debounceMax(debounceCount),
      _debounceCounter(0),
      _state(ALERT_NORMAL) {}

// ──────────────────────────────────────────────────────────────────────────
// Initialization
// ──────────────────────────────────────────────────────────────────────────

void ThresholdAlert::init() {
    _state = ALERT_NORMAL;
    _debounceCounter = 0;
}

// ──────────────────────────────────────────────────────────────────────────
// FSM Update — core threshold detection logic
// ──────────────────────────────────────────────────────────────────────────

AlertState ThresholdAlert::update(float value) {
    switch (_state) {

    // ── NORMAL: waiting for value to exceed high threshold ────────────
    case ALERT_NORMAL:
        if (value > _highThreshold) {
            // Value crossed the upper threshold — begin debounce.
            _state = ALERT_DEBOUNCE_HIGH;
            _debounceCounter = 1;
        }
        break;

    // ── DEBOUNCE_HIGH: counting consecutive above-threshold readings ─
    case ALERT_DEBOUNCE_HIGH:
        if (value > _highThreshold) {
            _debounceCounter++;
            if (_debounceCounter >= _debounceMax) {
                // Confirmed: alert is stable. Transition to ACTIVE.
                _state = ALERT_ACTIVE;
                _debounceCounter = 0;
            }
        } else {
            // Value dropped back — false alarm. Return to NORMAL.
            _state = ALERT_NORMAL;
            _debounceCounter = 0;
        }
        break;

    // ── ALERT_ACTIVE: waiting for value to drop below low threshold ──
    case ALERT_ACTIVE:
        if (value < _lowThreshold) {
            // Value crossed the lower threshold — begin debounce down.
            _state = ALERT_DEBOUNCE_LOW;
            _debounceCounter = 1;
        }
        break;

    // ── DEBOUNCE_LOW: counting consecutive below-threshold readings ──
    case ALERT_DEBOUNCE_LOW:
        if (value < _lowThreshold) {
            _debounceCounter++;
            if (_debounceCounter >= _debounceMax) {
                // Confirmed: value is stable below threshold. Clear alert.
                _state = ALERT_NORMAL;
                _debounceCounter = 0;
            }
        } else {
            // Value rose back — not a real recovery. Return to ALERT.
            _state = ALERT_ACTIVE;
            _debounceCounter = 0;
        }
        break;
    }

    return _state;
}

// ──────────────────────────────────────────────────────────────────────────
// State queries
// ──────────────────────────────────────────────────────────────────────────

AlertState ThresholdAlert::getState() const {
    return _state;
}

bool ThresholdAlert::isAlertActive() const {
    return _state == ALERT_ACTIVE;
}

bool ThresholdAlert::isDebouncing() const {
    return (_state == ALERT_DEBOUNCE_HIGH) || (_state == ALERT_DEBOUNCE_LOW);
}

uint8_t ThresholdAlert::getDebounceCounter() const {
    return _debounceCounter;
}

const char* ThresholdAlert::getStateString() const {
    return STATE_NAMES[_state];
}

// ──────────────────────────────────────────────────────────────────────────
// Runtime reconfiguration
// ──────────────────────────────────────────────────────────────────────────

void ThresholdAlert::setThresholds(float highThreshold, float lowThreshold) {
    _highThreshold = highThreshold;
    _lowThreshold  = lowThreshold;
    // Reset FSM to avoid inconsistent state with new thresholds.
    _state = ALERT_NORMAL;
    _debounceCounter = 0;
}

void ThresholdAlert::setDebounceCount(uint8_t count) {
    _debounceMax = count;
    _debounceCounter = 0;
}

float ThresholdAlert::getHighThreshold() const {
    return _highThreshold;
}

float ThresholdAlert::getLowThreshold() const {
    return _lowThreshold;
}
