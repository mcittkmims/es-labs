/**
 * @file StdioSerial.cpp
 * @brief STDIO-to-Serial Redirection Implementation
 *
 * Uses AVR libc's fdevopen/fdev_setup_stream to create a custom
 * FILE stream backed by the Arduino Serial (UART) port.
 *
 * - serialPutChar: writes a character to Serial (used by printf).
 * - serialGetChar: reads a character from Serial (used by fgets/scanf),
 *   with local echo and carriage-return-to-newline translation.
 */

#include "StdioSerial.h"

/**
 * @brief Write a single character to the serial port.
 *
 * Called internally by the C standard library whenever printf()
 * or putchar() outputs a character through stdout.
 *
 * @param c      The character to transmit.
 * @param stream Pointer to the FILE stream (unused).
 * @return 0 on success.
 */
static int serialPutChar(char c, FILE *stream) {
    Serial.write(c);
    return 0;
}

/**
 * @brief Read a single character from the serial port with echo.
 *
 * Called internally by the C standard library whenever fgets()
 * or scanf() reads a character from stdin. Blocks until a
 * character is available. Received characters are echoed back
 * to the terminal. Carriage return (\\r) is converted to
 * newline (\\n) for compatibility with fgets().
 *
 * @param stream Pointer to the FILE stream (unused).
 * @return The character read (as unsigned char cast to int).
 */
static int serialGetChar(FILE *stream) {
    // Block until data is available on the serial port
    while (!Serial.available()) {
        // Idle wait — acceptable for a simple polling-based application
    }

    char c = Serial.read();

    // Echo the received character back to the terminal
    if (c == '\r') {
        // Terminal sends \r on Enter; echo \r\n for visual newline
        Serial.write('\r');
        Serial.write('\n');
        return '\n'; // Return \n so fgets() recognizes end-of-line
    }

    // Handle backspace (ASCII 8 or DEL 127)
    if (c == '\b' || c == 127) {
        Serial.write('\b');
        Serial.write(' ');
        Serial.write('\b');
        return c;
    }

    Serial.write(c); // Echo printable characters
    return c;
}

/// Custom FILE stream that wraps the serial port for STDIO.
static FILE serialStream;

void stdioSerialInit(unsigned long baudRate) {
    Serial.begin(baudRate);

    // Wait for the serial port to be ready
    while (!Serial) {
        ; // Required for boards with native USB (e.g., Leonardo)
    }

    // Create a read-write stream backed by our put/get functions
    fdev_setup_stream(&serialStream, serialPutChar, serialGetChar, _FDEV_SETUP_RW);

    // Redirect standard C streams to use the serial port
    stdout = &serialStream;
    stdin  = &serialStream;
}
