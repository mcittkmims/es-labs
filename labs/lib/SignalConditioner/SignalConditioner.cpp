/**
 * @file SignalConditioner.cpp
 * @brief Signal Conditioning Pipeline Module Implementation
 *
 * Implements the three-stage signal conditioning pipeline:
 * saturation → median filter → EWMA. Each stage operates on
 * the output of the previous stage, producing a progressively
 * cleaner signal from noisy raw sensor data.
 *
 * The median filter uses insertion sort on a small array (max 9
 * elements), which is efficient for the small window sizes typical
 * in embedded sensor applications. The EWMA uses a single multiply-
 * accumulate operation per sample.
 */

#include "SignalConditioner.h"
#include <math.h>

// ──────────────────────────────────────────────────────────────────────────
// Constructor
// ──────────────────────────────────────────────────────────────────────────

SignalConditioner::SignalConditioner(uint8_t medianWindowSize, float ewmaAlpha,
                                     float minClamp, float maxClamp)
    : _windowSize(medianWindowSize > MAX_WINDOW_SIZE ? MAX_WINDOW_SIZE
                                                     : medianWindowSize),
      _count(0),
      _index(0),
      _ewmaAlpha(ewmaAlpha),
      _ewmaValue(0.0f),
      _ewmaInitialized(false),
      _minClamp(minClamp),
      _maxClamp(maxClamp),
      _lastRaw(0.0f),
      _lastSaturated(0.0f),
      _lastMedian(0.0f),
      _lastEwma(0.0f) {
    // Zero-initialize the window buffer.
    for (uint8_t i = 0; i < MAX_WINDOW_SIZE; i++) {
        _window[i] = 0.0f;
    }
}

// ──────────────────────────────────────────────────────────────────────────
// Full pipeline — saturate → median → EWMA
// ──────────────────────────────────────────────────────────────────────────

float SignalConditioner::process(float rawValue) {
    // ── Stage 1: Store raw value ────────────────────────────────────────
    _lastRaw = rawValue;

    // ── Stage 2: Saturation (clamping) ──────────────────────────────────
    // Handle NAN by clamping to the midpoint of the valid range,
    // preventing NAN from propagating through the pipeline.
    float saturated;
    if (isnan(rawValue)) {
        saturated = (_minClamp + _maxClamp) / 2.0f;
    } else {
        saturated = saturate(rawValue);
    }
    _lastSaturated = saturated;

    // ── Stage 3: Median filter ──────────────────────────────────────────
    // Insert the saturated value into the circular buffer.
    _window[_index] = saturated;
    _index = (_index + 1) % _windowSize;
    if (_count < _windowSize) {
        _count++;
    }

    // Compute median over the current buffer contents.
    _lastMedian = computeMedian();

    // ── Stage 4: EWMA ──────────────────────────────────────────────────
    if (!_ewmaInitialized) {
        // Seed the EWMA with the first median value.
        _ewmaValue = _lastMedian;
        _ewmaInitialized = true;
    } else {
        _ewmaValue = _ewmaAlpha * _lastMedian
                   + (1.0f - _ewmaAlpha) * _ewmaValue;
    }
    _lastEwma = _ewmaValue;

    return _lastEwma;
}

// ──────────────────────────────────────────────────────────────────────────
// Saturation (clamping)
// ──────────────────────────────────────────────────────────────────────────

float SignalConditioner::saturate(float value) const {
    if (value < _minClamp) return _minClamp;
    if (value > _maxClamp) return _maxClamp;
    return value;
}

// ──────────────────────────────────────────────────────────────────────────
// Median computation — insertion sort on a small local copy
// ──────────────────────────────────────────────────────────────────────────

float SignalConditioner::computeMedian() const {
    // Copy active elements to a local array for sorting.
    float sorted[MAX_WINDOW_SIZE];
    uint8_t n = _count;
    for (uint8_t i = 0; i < n; i++) {
        sorted[i] = _window[i];
    }

    // Insertion sort — O(n^2) but n <= 9, so this is fast.
    for (uint8_t i = 1; i < n; i++) {
        float key = sorted[i];
        int8_t j = i - 1;
        while (j >= 0 && sorted[j] > key) {
            sorted[j + 1] = sorted[j];
            j--;
        }
        sorted[j + 1] = key;
    }

    // Return the middle element (integer division rounds down for even n).
    return sorted[n / 2];
}

// ──────────────────────────────────────────────────────────────────────────
// Getters — intermediate values for diagnostic reporting
// ──────────────────────────────────────────────────────────────────────────

float SignalConditioner::getLastRaw() const {
    return _lastRaw;
}

float SignalConditioner::getLastSaturated() const {
    return _lastSaturated;
}

float SignalConditioner::getLastMedian() const {
    return _lastMedian;
}

float SignalConditioner::getLastEwma() const {
    return _lastEwma;
}

uint8_t SignalConditioner::getWindowSize() const {
    return _windowSize;
}

float SignalConditioner::getAlpha() const {
    return _ewmaAlpha;
}

float SignalConditioner::getMinClamp() const {
    return _minClamp;
}

float SignalConditioner::getMaxClamp() const {
    return _maxClamp;
}

bool SignalConditioner::isValid() const {
    return _count >= _windowSize;
}

uint8_t SignalConditioner::getSampleCount() const {
    return _count;
}

// ──────────────────────────────────────────────────────────────────────────
// Reset
// ──────────────────────────────────────────────────────────────────────────

void SignalConditioner::reset() {
    for (uint8_t i = 0; i < MAX_WINDOW_SIZE; i++) {
        _window[i] = 0.0f;
    }
    _count = 0;
    _index = 0;
    _ewmaValue = 0.0f;
    _ewmaInitialized = false;
    _lastRaw = 0.0f;
    _lastSaturated = 0.0f;
    _lastMedian = 0.0f;
    _lastEwma = 0.0f;
}
