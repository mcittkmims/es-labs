/**
 * @file SerialInput.h
 * @brief Serial Line Input Reader with Echo
 *
 * Provides a non-blocking line reader for the hardware serial port.
 * Accumulates characters into an internal buffer and returns a
 * complete line (null-terminated) when a newline or carriage return
 * is received. Characters are echoed back to the terminal as they
 * arrive.
 *
 * Usage:
 *   SerialInput input(32);
 *   input.init();
 *   const char *line = input.readLine();
 *   if (line) { processCommand(line); }
 */

#ifndef SERIAL_INPUT_H
#define SERIAL_INPUT_H

#include <Arduino.h>

/**
 * @class SerialInput
 * @brief Non-blocking serial line reader with character echo.
 */
class SerialInput {
public:
    /**
     * @brief Construct a new SerialInput object.
     * @param maxLen Maximum line length (excluding null terminator).
     *              Clamped to internal buffer size (63 chars max).
     */
    SerialInput(uint8_t maxLen = 31);

    /** @brief Reset the internal buffer index to prepare for new input. */
    void init();

    /**
     * @brief Attempt to read a complete line from the serial port.
     *
     * Reads all available characters, echoing each one back to the
     * terminal. When a newline or carriage return is received, the
     * accumulated string is null-terminated and returned.
     *
     * @return Pointer to the internal line buffer if a complete line
     *         is ready, or nullptr if still accumulating characters.
     */
    const char* readLine();

    /**
     * @brief Check if there are characters available on the serial port.
     * @return true if Serial.available() > 0.
     */
    bool available() const;

private:
    static const uint8_t MAX_BUF_SIZE = 64;
    char    _buf[MAX_BUF_SIZE]; ///< Internal character buffer
    uint8_t _maxLen;            ///< Configured max line length
    uint8_t _idx;               ///< Current write position in buffer
};

#endif // SERIAL_INPUT_H
