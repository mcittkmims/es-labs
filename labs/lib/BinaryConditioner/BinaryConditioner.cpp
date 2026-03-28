/**
 * @file BinaryConditioner.cpp
 * @brief Binary Signal Conditioning Implementation
 *
 * Implements the three-stage pipeline:
 *   1. Saturation   — coerces any input to strict boolean {false, true}.
 *   2. Debouncing   — requires stable input for debouncePeriodMs.
 *   3. Persistence  — requires confirmCount consecutive same-value samples.
 *
 * A state transition is committed only when BOTH conditions are met
 * simultaneously, preventing false transitions from noisy command input.
 */

#include "BinaryConditioner.h"

// ── Constructor ───────────────────────────────────────────────────────────────

BinaryConditioner::BinaryConditioner(uint32_t debouncePeriodMs, uint8_t persistCount)
    : _debouncePeriodMs(debouncePeriodMs),
      _persistCount(persistCount),
      _rawState(false),
      _pendingState(false),
      _conditionedState(false),
      _debounceStartMs(0),
      _inDebounce(false),
      _confirmCount(0)
{
}

// ── Core pipeline ─────────────────────────────────────────────────────────────

bool BinaryConditioner::process(bool rawInput) {
    // Stage 1: Saturation — coerce to strict {false, true}
    bool saturated = rawInput ? true : false;
    _rawState = saturated;

    // Track whether the incoming sample matches the pending candidate
    if (saturated != _pendingState) {
        // Input has switched — restart validation from scratch
        _pendingState    = saturated;
        _debounceStartMs = millis();
        _confirmCount    = 1;
    } else if (_pendingState != _conditionedState) {
        // Same pending, different from confirmed — accumulate confirmations
        _confirmCount++;
    }
    // If pending == conditioned, no transition is needed; leave count alone.

    // Stage 2 + 3: Evaluate transition when pending differs from confirmed
    if (_pendingState != _conditionedState) {
        _inDebounce = true;
        uint32_t elapsed = millis() - _debounceStartMs;

        // Commit the transition only when BOTH conditions are satisfied
        if (_confirmCount >= _persistCount && elapsed >= _debouncePeriodMs) {
            _conditionedState = _pendingState;
            _inDebounce       = false;
            _confirmCount     = 0;
        }
    } else {
        _inDebounce = false;
    }

    return _conditionedState;
}

// ── Accessors ─────────────────────────────────────────────────────────────────

bool    BinaryConditioner::getConditionedState()    const { return _conditionedState; }
bool    BinaryConditioner::getRawState()            const { return _rawState; }
bool    BinaryConditioner::getPendingState()        const { return _pendingState; }
bool    BinaryConditioner::isDebouncing()           const { return _inDebounce; }
uint8_t BinaryConditioner::getConfirmCount()        const { return _confirmCount; }
uint8_t BinaryConditioner::getRequiredConfirmCount()const { return _persistCount; }

// ── Reset ─────────────────────────────────────────────────────────────────────

void BinaryConditioner::reset() {
    _rawState         = false;
    _pendingState     = false;
    _conditionedState = false;
    _debounceStartMs  = 0;
    _inDebounce       = false;
    _confirmCount     = 0;
}
