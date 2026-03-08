/**
 * @file ThresholdAlert.h
 * @brief Hysteresis-Based Threshold Alert Module Interface
 *
 * Provides a reusable signal conditioning module that detects when a
 * measured value crosses configurable thresholds. Implements hysteresis
 * (dual thresholds) to prevent oscillation and software debouncing
 * (persistence counting) to ensure stable alert transitions.
 *
 * The module operates as a 4-state FSM:
 *
 *   NORMAL ──(value > highThreshold)──> DEBOUNCE_HIGH
 *   DEBOUNCE_HIGH ──(N confirmations)──> ALERT
 *   DEBOUNCE_HIGH ──(value < highThreshold before N)──> NORMAL
 *   ALERT ──(value < lowThreshold)──> DEBOUNCE_LOW
 *   DEBOUNCE_LOW ──(N confirmations)──> NORMAL
 *   DEBOUNCE_LOW ──(value > lowThreshold before N)──> ALERT
 *
 * Hysteresis band: [lowThreshold, highThreshold]
 *   - Alert triggers when value rises above highThreshold
 *   - Alert clears when value drops below lowThreshold
 *   - Values between low and high do not change the state
 *
 * Debounce: the state transition only completes after N consecutive
 * readings confirm the condition, preventing transient spikes from
 * triggering false alerts.
 *
 * Usage:
 *   ThresholdAlert alert(30.0, 28.0, 5);  // high=30, low=28, 5 confirmations
 *   alert.init();
 *   AlertState state = alert.update(currentTemp);
 */

#ifndef THRESHOLD_ALERT_H
#define THRESHOLD_ALERT_H

#include <Arduino.h>

/**
 * @enum AlertState
 * @brief States of the threshold alert FSM.
 */
enum AlertState {
    ALERT_NORMAL,         /**< Value is below the low threshold — no alert.       */
    ALERT_DEBOUNCE_HIGH,  /**< Value crossed high threshold — debouncing upward.  */
    ALERT_ACTIVE,         /**< Alert confirmed — value above threshold (stable).  */
    ALERT_DEBOUNCE_LOW    /**< Value crossed low threshold — debouncing downward. */
};

/**
 * @class ThresholdAlert
 * @brief Hysteresis-based threshold detector with debounce filtering.
 *
 * Monitors a floating-point input value and transitions between NORMAL
 * and ALERT states using configurable hysteresis and debounce parameters.
 */
class ThresholdAlert {
public:
    /**
     * @brief Construct a new ThresholdAlert object.
     *
     * @param highThreshold Value above which the alert begins to trigger.
     * @param lowThreshold  Value below which the alert begins to clear.
     *                      Must be less than highThreshold.
     * @param debounceCount Number of consecutive confirmations required
     *                      to complete a state transition (default: 5).
     */
    ThresholdAlert(float highThreshold, float lowThreshold,
                   uint8_t debounceCount = 5);

    /**
     * @brief Initialize (reset) the alert module to NORMAL state.
     */
    void init();

    /**
     * @brief Update the alert FSM with a new sensor reading.
     *
     * Call this method at regular intervals (e.g., every 100 ms).
     * The returned state indicates the current alert condition.
     *
     * @param value The current sensor reading.
     * @return AlertState The current state of the alert FSM.
     */
    AlertState update(float value);

    /**
     * @brief Get the current alert state without updating.
     * @return AlertState Current FSM state.
     */
    AlertState getState() const;

    /**
     * @brief Check if an alert is currently active.
     * @return true if the state is ALERT_ACTIVE.
     */
    bool isAlertActive() const;

    /**
     * @brief Check if a state transition is in progress (debouncing).
     * @return true if the state is DEBOUNCE_HIGH or DEBOUNCE_LOW.
     */
    bool isDebouncing() const;

    /**
     * @brief Get the current debounce counter value.
     * @return uint8_t Current count of consecutive confirmations.
     */
    uint8_t getDebounceCounter() const;

    /**
     * @brief Get a human-readable string representation of the current state.
     * @return const char* State name string.
     */
    const char* getStateString() const;

    /**
     * @brief Reconfigure the threshold values at runtime.
     *
     * Resets the FSM to NORMAL state after reconfiguration.
     *
     * @param highThreshold New high threshold.
     * @param lowThreshold  New low threshold (must be < highThreshold).
     */
    void setThresholds(float highThreshold, float lowThreshold);

    /**
     * @brief Reconfigure the debounce count at runtime.
     *
     * Resets the debounce counter but does not change the FSM state.
     *
     * @param count New debounce confirmation count.
     */
    void setDebounceCount(uint8_t count);

    /**
     * @brief Get the configured high threshold.
     * @return float High threshold value.
     */
    float getHighThreshold() const;

    /**
     * @brief Get the configured low threshold.
     * @return float Low threshold value.
     */
    float getLowThreshold() const;

private:
    float      _highThreshold;   /**< Upper threshold for alert trigger.   */
    float      _lowThreshold;    /**< Lower threshold for alert clear.     */
    uint8_t    _debounceMax;     /**< Number of confirmations required.     */
    uint8_t    _debounceCounter; /**< Current consecutive confirmation count. */
    AlertState _state;           /**< Current FSM state.                    */
};

#endif // THRESHOLD_ALERT_H
