/**
 * @file SignalConditioner.h
 * @brief Signal Conditioning Pipeline Module Interface
 *
 * Provides a reusable signal conditioning pipeline that processes raw
 * sensor readings through three sequential stages:
 *
 *   1. Saturation (clamping): constrains the input to a valid range,
 *      rejecting physically impossible readings (e.g., temperatures
 *      outside the sensor's operating range).
 *
 *   2. Median filter: removes impulsive noise ("salt and pepper")
 *      by maintaining a sliding window of recent samples and
 *      selecting the median value. Effective against single-sample
 *      spikes caused by electrical interference or sensor glitches.
 *
 *   3. Exponentially Weighted Moving Average (EWMA): smooths the
 *      median-filtered signal using a first-order IIR filter.
 *      The smoothing factor alpha controls the trade-off between
 *      responsiveness (high alpha) and noise reduction (low alpha).
 *
 * Pipeline:
 *   raw → saturate → median → EWMA → conditioned output
 *
 * All intermediate values are stored and accessible via getters,
 * enabling diagnostic reporting of each pipeline stage.
 *
 * Usage:
 *   SignalConditioner cond(5, 0.3, -40.0, 125.0);
 *   float conditioned = cond.process(rawTemperature);
 *   float median = cond.getLastMedian();
 */

#ifndef SIGNAL_CONDITIONER_H
#define SIGNAL_CONDITIONER_H

#include <Arduino.h>

/**
 * @class SignalConditioner
 * @brief Multi-stage signal conditioning pipeline (saturate → median → EWMA).
 *
 * Encapsulates a configurable conditioning pipeline for continuous sensor
 * readings. The median filter uses a fixed-size circular buffer (max 9
 * samples) to avoid dynamic memory allocation on resource-constrained MCUs.
 */
class SignalConditioner {
public:
    /**
     * @brief Construct a new SignalConditioner object.
     *
     * @param medianWindowSize Number of samples in the median filter window
     *                         (must be odd and <= MAX_WINDOW_SIZE, default 5).
     * @param ewmaAlpha        EWMA smoothing factor (0.0–1.0). Higher values
     *                         give more weight to recent samples.
     * @param minClamp         Minimum valid value (saturation lower bound).
     * @param maxClamp         Maximum valid value (saturation upper bound).
     */
    SignalConditioner(uint8_t medianWindowSize, float ewmaAlpha,
                      float minClamp, float maxClamp);

    /**
     * @brief Process a raw sensor reading through the full pipeline.
     *
     * Applies saturation, median filtering, and EWMA in sequence.
     * Returns the final conditioned (EWMA) value.
     *
     * @param rawValue The raw sensor reading to process.
     * @return float The conditioned output value (after EWMA).
     */
    float process(float rawValue);

    /**
     * @brief Get the last raw (unprocessed) input value.
     * @return float Last raw value passed to process().
     */
    float getLastRaw() const;

    /**
     * @brief Get the last saturated value (after clamping, before median).
     * @return float Last saturated value.
     */
    float getLastSaturated() const;

    /**
     * @brief Get the last median-filtered value (after median, before EWMA).
     * @return float Last median value.
     */
    float getLastMedian() const;

    /**
     * @brief Get the last EWMA value (final conditioned output).
     * @return float Last EWMA value.
     */
    float getLastEwma() const;

    /**
     * @brief Get the configured median window size.
     * @return uint8_t Window size.
     */
    uint8_t getWindowSize() const;

    /**
     * @brief Get the configured EWMA alpha parameter.
     * @return float Alpha value.
     */
    float getAlpha() const;

    /**
     * @brief Get the configured minimum saturation bound.
     * @return float Minimum clamp value.
     */
    float getMinClamp() const;

    /**
     * @brief Get the configured maximum saturation bound.
     * @return float Maximum clamp value.
     */
    float getMaxClamp() const;

    /**
     * @brief Check if the conditioning pipeline is fully primed.
     *
     * Returns true once the median window has been completely filled
     * with samples. Before this point, the median is computed over
     * a partial window and the EWMA may not be fully stabilized.
     *
     * @return true if the window is full (sample count >= window size).
     */
    bool isValid() const;

    /**
     * @brief Get the number of samples received so far.
     * @return uint8_t Sample count (saturates at window size).
     */
    uint8_t getSampleCount() const;

    /**
     * @brief Reset the conditioner to its initial state.
     *
     * Clears the median window buffer and resets the EWMA state.
     * The next call to process() will begin filling the window fresh.
     */
    void reset();

private:
    /** Maximum supported median window size (fixed array, no heap). */
    static const uint8_t MAX_WINDOW_SIZE = 9;

    // ── Median filter state ─────────────────────────────────────────────
    float   _window[MAX_WINDOW_SIZE]; /**< Circular buffer for median.    */
    uint8_t _windowSize;              /**< Configured window size.        */
    uint8_t _count;                   /**< Samples received (0..windowSz).*/
    uint8_t _index;                   /**< Next write position in buffer. */

    // ── EWMA state ──────────────────────────────────────────────────────
    float _ewmaAlpha;                 /**< Smoothing factor (0.0–1.0).    */
    float _ewmaValue;                 /**< Current EWMA accumulator.      */
    bool  _ewmaInitialized;           /**< True after first sample.       */

    // ── Saturation bounds ───────────────────────────────────────────────
    float _minClamp;                  /**< Lower saturation bound.        */
    float _maxClamp;                  /**< Upper saturation bound.        */

    // ── Intermediate values for diagnostic reporting ────────────────────
    float _lastRaw;                   /**< Last raw input.                */
    float _lastSaturated;             /**< After saturation.              */
    float _lastMedian;                /**< After median filter.           */
    float _lastEwma;                  /**< After EWMA (final output).     */

    /**
     * @brief Clamp a value to the configured [min, max] range.
     * @param value Input value.
     * @return float Clamped value.
     */
    float saturate(float value) const;

    /**
     * @brief Compute the median of the current window contents.
     *
     * Copies the active portion of the circular buffer to a local
     * array, sorts it with insertion sort, and returns the middle
     * element. For even-count windows, returns the lower-middle value.
     *
     * @return float Median of the current window.
     */
    float computeMedian() const;
};

#endif // SIGNAL_CONDITIONER_H
