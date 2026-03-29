/**
 * @file SerialInput.cpp
 * @brief Serial Line Input Reader Implementation
 *
 * Wraps Arduino Serial.available(), Serial.read(), and Serial.write()
 * behind a line-oriented interface. All low-level serial I/O is
 * encapsulated here so that application code never calls Serial
 * methods directly.
 */

#include "SerialInput.h"

SerialInput::SerialInput(uint8_t maxLen)
    : _maxLen(maxLen < MAX_BUF_SIZE ? maxLen : MAX_BUF_SIZE - 1),
      _idx(0) {
    _buf[0] = '\0';
}

void SerialInput::init() {
    _idx = 0;
    _buf[0] = '\0';
}

const char* SerialInput::readLine() {
    while (Serial.available() > 0) {
        char c = (char)Serial.read();
        // Echo character back to terminal
        Serial.write(c);

        if (c == '\n' || c == '\r') {
            if (_idx > 0) {
                _buf[_idx] = '\0';
                _idx = 0;
                return _buf;
            }
        } else if (_idx < _maxLen) {
            _buf[_idx++] = c;
        }
    }
    return nullptr;
}

bool SerialInput::available() const {
    return Serial.available() > 0;
}
