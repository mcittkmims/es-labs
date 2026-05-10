/**
 * @file lab6_1_main.cpp
 * @brief Lab 6.1 Entry Point Implementation
 *
 * Laboratory Work 6.1: "Behavioural control with Finite State Machines
 * --- Button-LED" (manual section 7.2.1).
 *
 * The application demonstrates a textbook Moore FSM with two states
 * (LED_OFF, LED_ON). The state transition is triggered exclusively by
 * a clean, debounced rising edge on a push button. The current FSM
 * state is mirrored on a digital LED and reported over the STDIO serial
 * interface using the C standard library (printf).
 *
 * Architecture (layered, low-level access encapsulated in libraries):
 *
 *   APP   -- lab6_1Setup() / lab6_1Loop()       (orchestration only)
 *   SRV   -- ButtonLedFsm                       (state table, transitions)
 *   ECAL  -- StdioSerial                        (printf -> UART)
 *   MCAL  -- Button (GPIO + debounce), Led      (GPIO write)
 *   HW    -- ATmega2560 GPIO peripherals
 *
 * No digitalRead/digitalWrite/Serial.* calls appear in this file: every
 * hardware interaction is performed by the corresponding library object.
 *
 * Hardware Configuration:
 *   - MCU       : Arduino Mega 2560
 *   - Button    : digital pin 7, active-LOW (INPUT_PULLUP), debounce 30 ms
 *   - LED       : digital pin 8, with 220 Ohm series resistor to ground
 *   - Serial    : UART0 at 9600 baud (USB-to-serial)
 *
 * The debounce window is comfortably below the 100 ms response budget
 * stated in the manual (RNF01).
 */

#include "lab6_1_main.h"

#include <Arduino.h>
#include <stdio.h>

#include "Button.h"
#include "ButtonLedFsm.h"
#include "Led.h"
#include "StdioSerial.h"

// ============================================================
// Pin / configuration constants (single source of truth)
// ============================================================

/// GPIO pin connected to the push button (active-LOW with INPUT_PULLUP).
static const uint8_t PIN_BUTTON = 7;

/// GPIO pin driving the controlled LED (HIGH = on).
static const uint8_t PIN_LED = 8;

/// UART baud rate for the STDIO serial bridge.
static const unsigned long BAUD_RATE = 9600;

/// Stable-time threshold (ms) used by the Button debounce filter.
static const uint16_t BUTTON_DEBOUNCE_MS = 30;

/**
 * @brief Cooperative loop period (ms).
 *
 * The loop polls the button at this cadence. With BUTTON_DEBOUNCE_MS = 30
 * and a poll period of 5 ms, the worst-case response from physical press
 * to confirmed FSM transition stays well under 100 ms.
 */
static const uint16_t LOOP_TICK_MS = 5;

// ============================================================
// Module-level driver instances
// ============================================================

static Button       button(PIN_BUTTON, /*activeLow=*/true, BUTTON_DEBOUNCE_MS);
static Led          led(PIN_LED);
static ButtonLedFsm fsm;

// ============================================================
// Helpers
// ============================================================

/**
 * @brief Print the FSM state table (Tabelul 7.1 from the manual).
 *
 * This is purely informational and runs once at startup; it documents,
 * on the user's own terminal, the deterministic behaviour of the FSM.
 */
static void printFsmTable() {
    printf("FSM state table (Moore, 2 states):\r\n");
    printf("  Num | Name    | Out | In=0    | In=1    \r\n");
    printf("  ----+---------+-----+---------+---------\r\n");
    printf("  0   | LED_OFF |  0  | LED_OFF | LED_ON  \r\n");
    printf("  1   | LED_ON  |  1  | LED_ON  | LED_OFF \r\n");
}

// ============================================================
// Public API Implementation
// ============================================================

void lab6_1Setup() {
    // Bring up STDIO over UART so printf works for diagnostics.
    stdioSerialInit(BAUD_RATE);

    // Initialize the hardware drivers (encapsulate all pinMode/digitalRead).
    button.init();
    led.init();

    // Reset the FSM to its initial state and apply the Moore output.
    fsm.init();
    led.set(fsm.getOutput() != 0);

    printf("\r\n");
    printf("========================================\r\n");
    printf("  Lab 6.1: Button-LED Finite State       \r\n");
    printf("           Machine (Moore, 2 states)    \r\n");
    printf("  MCU: Arduino Mega 2560                 \r\n");
    printf("========================================\r\n");
    printf("Wiring:\r\n");
    printf("  Button -> D%u (active-LOW, INPUT_PULLUP)\r\n", PIN_BUTTON);
    printf("  LED    -> D%u (220 Ohm to GND)\r\n",            PIN_LED);
    printf("Debounce window: %u ms\r\n", (unsigned)BUTTON_DEBOUNCE_MS);
    printf("\r\n");
    printFsmTable();
    printf("\r\n");
    printf("[FSM] Initial state: %s\r\n", fsm.getStateName());
    printf("Press the button to toggle the LED state.\r\n");
    printf("\r\n");

    // The constructor sets _changed = true so the LED was synchronized
    // above; clear the flag to avoid re-printing on the first loop pass.
    fsm.clearChanged();
}

void lab6_1Loop() {
    // Step 1 -- Read input (debounced) ---------------------------------
    button.update();

    // Step 2 -- Drive the FSM with confirmed press events --------------
    if (button.wasPressed()) {
        fsm.processEvent();
    }

    // Step 3 -- Apply Moore output + report state changes --------------
    if (fsm.changed()) {
        led.set(fsm.getOutput() != 0);
        printf("[FSM] State -> %s (LED %s)\r\n",
               fsm.getStateName(),
               led.isOn() ? "ON" : "OFF");
        fsm.clearChanged();
    }

    // Step 4 -- Cooperative wait so other software can share the CPU --
    delay(LOOP_TICK_MS);
}
