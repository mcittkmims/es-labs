/**
 * @file StdioSerial.h
 * @brief STDIO-to-Serial Redirection Interface
 *
 * Redirects the standard C library I/O streams (stdout, stdin)
 * to the hardware UART serial port. After initialization, standard
 * functions like printf() and fgets() communicate over the serial
 * interface, enabling text-based user interaction via a terminal.
 *
 * Usage:
 *   stdioSerialInit(9600);
 *   printf("Hello from STDIO!\r\n");
 *   fgets(buffer, sizeof(buffer), stdin);
 */

#ifndef STDIO_SERIAL_H
#define STDIO_SERIAL_H

#include <Arduino.h>
#include <stdio.h>

/**
 * @brief Initialize STDIO redirection over the hardware serial port.
 *
 * Configures the UART at the specified baud rate and redirects
 * stdout and stdin to use the serial port. Characters received
 * via stdin are echoed back to the terminal for user feedback.
 *
 * @param baudRate The serial communication baud rate (e.g., 9600).
 */
void stdioSerialInit(unsigned long baudRate);

#endif // STDIO_SERIAL_H
