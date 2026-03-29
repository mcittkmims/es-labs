/**
 * @file BinaryConditioner.h
 * @brief Binary Signal Conditioner (Counter-Based Debounce)
 *
 * Implements counter-based debouncing for binary actuator commands.
 * When a new command is received (ON or OFF), the conditioner requires
 * a configurable number of consecutive confirmations before accepting
 * the state change. This prevents false triggering from noisy inputs
 * or accidental rapid toggling.
 *
 * Algorithm:
 *   - A counter increments toward MAX when input is HIGH and
 *     decrements toward 0 when input is LOW.
 *   - Output transitions to ON only when counter reaches MAX.
 *   - Output transitions to OFF only when counter reaches 0.
 *   - Intermediate counter values retain the previous stable output.
 *
 * Usage:
 *   BinaryConditioner cond(5);  // 5 confirmations required
 *   cond.init();
 *   bool stable = cond.process(rawInput);
 */

#ifndef BINARY_CONDITIONER_H
#define BINARY_CONDITIONER_H

#include <Arduino.h>

/**
 * @class BinaryConditioner
 * @brief Counter-based debounce filter for binary signals.
 */
class BinaryConditioner {
public:
    /**
     * @brief Construct a new BinaryConditioner.
     * @param maxCount Number of consecutive confirmations required
     *                 for a state transition (default: 5).
     */
    BinaryConditioner(uint8_t maxCount = 5);

    /** @brief Reset the conditioner to OFF state with counter at 0. */
    void init();

    /**
     * @brief Process a raw binary input through the debounce filter.
     *
     * Call this at regular intervals (e.g., every 50--100 ms).
     * The method returns the stable (debounced) output state.
     *
     * @param rawInput The raw binary command (true = ON requested).
     * @return bool The debounced output state.
     */
    bool process(bool rawInput);

    /** @brief Get the current debounced output state. */
    bool getState() const;

    /** @brief Get the current counter value (for diagnostics). */
    uint8_t getCounter() const;

    /** @brief Get the configured maximum count. */
    uint8_t getMaxCount() const;

    /** @brief Check if the output is currently transitioning. */
    bool isTransitioning() const;

private:
    uint8_t _maxCount;  ///< Confirmation threshold
    uint8_t _counter;   ///< Current debounce counter
    bool    _output;    ///< Current stable output state
};

#endif // BINARY_CONDITIONER_H
