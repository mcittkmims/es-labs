/**
 * @file BinaryConditioner.h
 * @brief Binary Signal Conditioning Interface
 *
 * Applies a three-stage conditioning pipeline to a binary (boolean) signal:
 *
 *   Stage 1 — Saturation: maps any non-zero input to logical true, ensuring
 *             the value is strictly in {false, true}.
 *
 *   Stage 2 — Debouncing: requires the input to remain stable for at least
 *             debouncePeriodMs before accepting a state transition.
 *
 *   Stage 3 — Persistent Validation: requires at least persistCount
 *             consecutive identical samples before committing a state change.
 *
 * Both Stage 2 and Stage 3 must be satisfied simultaneously for a transition
 * to be accepted. This prevents spurious ON/OFF commands caused by rapid or
 * accidental input from reaching the actuator hardware.
 *
 * Usage:
 *   BinaryConditioner cond(100, 3);   // 100 ms debounce, 3 confirmations
 *   bool validated = cond.process(rawBool);
 */

#ifndef BINARY_CONDITIONER_H
#define BINARY_CONDITIONER_H

#include <Arduino.h>

/**
 * @class BinaryConditioner
 * @brief Three-stage binary signal conditioner: saturation → debounce → persistence.
 */
class BinaryConditioner {
public:
    /**
     * @brief Construct a BinaryConditioner.
     * @param debouncePeriodMs Minimum stable duration before accepting a change (ms).
     * @param persistCount     Minimum consecutive same-value samples required.
     */
    BinaryConditioner(uint32_t debouncePeriodMs = 100, uint8_t persistCount = 3);

    /**
     * @brief Process one raw sample through the conditioning pipeline.
     *
     * Must be called at a fixed period (typically 50–100 ms).
     * A state transition is committed only when both debounce period and
     * persistence count have been satisfied.
     *
     * @param rawInput Incoming raw boolean value.
     * @return The conditioned (validated) output state.
     */
    bool process(bool rawInput);

    /** @brief Returns the last conditioned (validated) output state. */
    bool getConditionedState() const;

    /** @brief Returns the last raw (saturated) input value. */
    bool getRawState() const;

    /** @brief Returns the pending state currently being validated. */
    bool getPendingState() const;

    /** @brief Returns true if a state transition is being validated. */
    bool isDebouncing() const;

    /** @brief Returns the current consecutive confirmation count. */
    uint8_t getConfirmCount() const;

    /** @brief Returns the required number of consecutive confirmations. */
    uint8_t getRequiredConfirmCount() const;

    /** @brief Reset to initial state (all false, no pending transition). */
    void reset();

private:
    uint32_t _debouncePeriodMs;  ///< Minimum debounce window (ms)
    uint8_t  _persistCount;       ///< Required consecutive confirmations

    bool     _rawState;           ///< Last saturated input
    bool     _pendingState;       ///< State being validated
    bool     _conditionedState;   ///< Last accepted (committed) output

    uint32_t _debounceStartMs;    ///< millis() timestamp when current pending started
    bool     _inDebounce;         ///< True if a transition is in progress
    uint8_t  _confirmCount;       ///< Consecutive confirmations so far
};

#endif // BINARY_CONDITIONER_H
