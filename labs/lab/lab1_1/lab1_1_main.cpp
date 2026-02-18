/**
 * @file lab1_1_main.cpp
 * @brief Lab 1.1 Entry Point Implementation
 *
 * Laboratory Work 1.1: "User Interaction: STDIO - Serial Interface"
 *
 * This module implements a simple serial command interface that allows
 * the user to control an LED via text commands entered in a terminal:
 *   - "led on"  -> turns the LED ON
 *   - "led off" -> turns the LED OFF
 *
 * Communication is handled entirely through the C standard I/O library
 * (printf for output, fgets for input), redirected to the hardware UART.
 *
 * Hardware Configuration:
 *   - MCU: Arduino Mega 2560
 *   - LED: connected to digital pin 7 via a 220 Ohm resistor
 *   - Serial: UART0 at 9600 baud
 */

#include "lab1_1_main.h"
#include <Arduino.h>
#include <stdio.h>
#include "Led.h"
#include "StdioSerial.h"
#include "CommandParser.h"

// ============================================================
// Pin Configuration (single source of truth for hardware mapping)
// ============================================================

/// GPIO pin connected to the external LED (via 220 Ohm resistor).
static const uint8_t LED_PIN = 7;

/// UART baud rate for serial communication.
static const unsigned long BAUD_RATE = 9600;

// ============================================================
// Module-level objects
// ============================================================

/// LED driver instance, bound to LED_PIN.
static Led led(LED_PIN);

/// Input buffer for receiving serial commands.
static char inputBuffer[64];

// ============================================================
// Public API Implementation
// ============================================================

void lab1_1Setup() {
    // Initialize STDIO over Serial (redirects printf/fgets to UART)
    stdioSerialInit(BAUD_RATE);

    // Initialize the LED hardware (pin mode + default OFF state)
    led.init();

    // Display the welcome banner and usage instructions
    printf("\r\n");
    printf("========================================\r\n");
    printf("  Lab 1.1: Serial LED Control (STDIO)\r\n");
    printf("  MCU: Arduino Mega 2560\r\n");
    printf("========================================\r\n");
    printf("\r\n");
    printf("Available commands:\r\n");
    printf("  led on   - Turn the LED ON\r\n");
    printf("  led off  - Turn the LED OFF\r\n");
    printf("\r\n");
}

void lab1_1Loop() {
    // Prompt the user for input
    printf("> ");

    // Read a line of text from stdin (blocks until Enter is pressed)
    if (fgets(inputBuffer, sizeof(inputBuffer), stdin) != NULL) {
        // Parse the received command string
        CommandType cmd = parseCommand(inputBuffer);

        // Execute the command and provide feedback
        switch (cmd) {
            case CMD_LED_ON:
                led.turnOn();
                printf("[OK] LED is now ON.\r\n");
                break;

            case CMD_LED_OFF:
                led.turnOff();
                printf("[OK] LED is now OFF.\r\n");
                break;

            case CMD_UNKNOWN:
                printf("[ERROR] Unknown command.\r\n");
                printf("Use 'led on' or 'led off'.\r\n");
                break;
        }
    }
}
