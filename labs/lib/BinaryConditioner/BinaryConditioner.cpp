/**
 * @file BinaryConditioner.cpp
 * @brief Binary Signal Conditioner Implementation
 *
 * Implements counter-based debouncing as described in the lab
 * methodology (Section 4.1.6): the counter increments when input
 * is HIGH and decrements when LOW, with state transitions only
 * occurring at the counter boundaries (0 and MAX).
 */

#include "BinaryConditioner.h"

BinaryConditioner::BinaryConditioner(uint8_t maxCount)
    : _maxCount(maxCount), _counter(0), _output(false) {}

void BinaryConditioner::init() {
    _counter = 0;
    _output = false;
}

bool BinaryConditioner::process(bool rawInput) {
    // Increment or decrement counter based on raw input
    if (rawInput) {
        if (_counter < _maxCount) {
            _counter++;
        }
    } else {
        if (_counter > 0) {
            _counter--;
        }
    }

    // State transitions only at boundaries
    if (_counter >= _maxCount) {
        _output = true;
    } else if (_counter == 0) {
        _output = false;
    }
    // Intermediate values: retain previous output

    return _output;
}

bool BinaryConditioner::getState() const {
    return _output;
}

uint8_t BinaryConditioner::getCounter() const {
    return _counter;
}

uint8_t BinaryConditioner::getMaxCount() const {
    return _maxCount;
}

bool BinaryConditioner::isTransitioning() const {
    return (_counter > 0 && _counter < _maxCount);
}
