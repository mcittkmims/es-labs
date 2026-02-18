/**
 * @file lab1_2_main.cpp
 * @brief Lab 1.2 Entry Point Implementation
 *
 * Laboratory Work 1.2: "User Interaction: LCD + Keypad"
 *
 * This module implements an electronic lock system with a menu-driven
 * interface using a 4x4 matrix keypad for input and a 16x2 I2C LCD
 * for output. The system is controlled by a finite state machine (FSM)
 * that processes keypad commands in the format:
 *
 *   *0#          - Lock unconditionally
 *   *1*password# - Unlock with password verification
 *   *2*old*new#  - Change password
 *   *3#          - Display current lock status
 *
 * Visual feedback is provided through:
 *   - LCD: context-aware menus and confirmation messages
 *   - Red LED: indicates LOCKED state
 *   - Green LED: indicates UNLOCKED state
 *   - Serial (STDIO): debug logging of key presses and state transitions
 *
 * Hardware Configuration:
 *   - MCU: Arduino Mega 2560
 *   - LCD: 16x2 I2C at address 0x27 (SDA=20, SCL=21)
 *   - Keypad: 4x4 membrane (rows=22-25, cols=26-29)
 *   - Red LED: pin 7 via 220 Ohm resistor (locked indicator)
 *   - Green LED: pin 6 via 220 Ohm resistor (unlocked indicator)
 *   - Serial: UART0 at 9600 baud (debug output)
 */

#include "lab1_2_main.h"
#include <Arduino.h>
#include <stdio.h>
#include "Led.h"
#include "LcdDisplay.h"
#include "KeypadInput.h"
#include "LockFSM.h"
#include "StdioSerial.h"

// ============================================================
// Pin Configuration (single source of truth for hardware mapping)
// ============================================================

/// GPIO pin for the red LED (locked indicator), via 220 Ohm resistor.
static const uint8_t RED_LED_PIN = 7;

/// GPIO pin for the green LED (unlocked indicator), via 220 Ohm resistor.
static const uint8_t GREEN_LED_PIN = 6;

/// I2C address of the LCD1602 display module.
static const uint8_t LCD_I2C_ADDR = 0x27;

/// Number of LCD columns.
static const uint8_t LCD_COLS = 16;

/// Number of LCD rows.
static const uint8_t LCD_ROWS = 2;

/// UART baud rate for serial debug output.
static const unsigned long BAUD_RATE = 9600;

/// Keypad row GPIO pins (directly connected to membrane keypad rows).
static byte rowPins[4] = {22, 23, 24, 25};

/// Keypad column GPIO pins (directly connected to membrane keypad columns).
static byte colPins[4] = {26, 27, 28, 29};

// ============================================================
// Module-level Objects
// ============================================================

/// Red LED driver instance (locked indicator).
static Led redLed(RED_LED_PIN);

/// Green LED driver instance (unlocked indicator).
static Led greenLed(GREEN_LED_PIN);

/// LCD display driver instance (I2C, 16x2).
static LcdDisplay lcd(LCD_I2C_ADDR, LCD_COLS, LCD_ROWS);

/// Keypad input driver instance (4x4 membrane).
static KeypadInput keypad(rowPins, colPins);

/// Lock finite state machine instance.
static LockFSM lockFSM;

/// Tracks the previous lock state to detect LED transitions.
static bool prevLocked = true;

// ============================================================
// Public API Implementation
// ============================================================

void lab1_2Setup() {
    // Initialize STDIO over Serial (for debug printf output)
    stdioSerialInit(BAUD_RATE);

    // Initialize peripherals
    lcd.init();
    keypad.init();
    redLed.init();
    greenLed.init();

    // Initialize the lock FSM (starts in LOCKED state)
    lockFSM.init();

    // Set initial LED state: locked (red ON, green OFF)
    redLed.turnOn();
    greenLed.turnOff();
    prevLocked = true;

    // Display initial FSM state on LCD
    const LockDisplay& disp = lockFSM.getDisplay();
    lcd.showTwoLines(disp.line1, disp.line2);
    lockFSM.clearDisplayChanged();

    // Print welcome banner to serial terminal
    printf("\r\n");
    printf("========================================\r\n");
    printf("  Lab 1.2: LCD + Keypad Lock System\r\n");
    printf("  MCU: Arduino Mega 2560\r\n");
    printf("========================================\r\n");
    printf("\r\n");
    printf("Commands (via keypad):\r\n");
    printf("  *0#          - Lock\r\n");
    printf("  *1*pwd#      - Unlock with password\r\n");
    printf("  *2*old*new#  - Change password\r\n");
    printf("  *3#          - Show lock status\r\n");
    printf("\r\n");
    printf("Default password: 1234\r\n");
    printf("\r\n");
}

void lab1_2Loop() {
    // --- 1. Read keypad input (non-blocking) ---
    char key = keypad.getKey();
    if (key) {
        lockFSM.processKey(key);
    }

    // --- 2. Handle timed transitions (result display timeout) ---
    lockFSM.update();

    // --- 3. Update LCD when display content changes ---
    if (lockFSM.displayChanged()) {
        const LockDisplay& disp = lockFSM.getDisplay();
        lcd.showTwoLines(disp.line1, disp.line2);
        lockFSM.clearDisplayChanged();
    }

    // --- 4. Update LED indicators on lock state change ---
    bool currentLocked = lockFSM.isLocked();
    if (currentLocked != prevLocked) {
        if (currentLocked) {
            redLed.turnOn();
            greenLed.turnOff();
            printf("[LED] Red ON, Green OFF (LOCKED)\r\n");
        } else {
            redLed.turnOff();
            greenLed.turnOn();
            printf("[LED] Red OFF, Green ON (UNLOCKED)\r\n");
        }
        prevLocked = currentLocked;
    }
}
