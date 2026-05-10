/**
 * @file ButtonLedFsm.h
 * @brief Button-LED Finite State Machine Interface (Moore Automaton)
 *
 * Implements the table-driven Moore FSM described in Section 7.1 of the
 * laboratory manual ("Lucrarea de laborator nr. 7: Control comportamental
 * cu automate finite"). The FSM has exactly two stable states:
 *
 *   - LED_OFF_STATE : the controlled LED is OFF.
 *   - LED_ON_STATE  : the controlled LED is ON.
 *
 * On each rising edge of the input signal (a confirmed, debounced button
 * press) the automaton toggles between these two states, producing the
 * canonical "press to toggle" behaviour. Following the Moore convention,
 * the output value depends only on the current state, not on the input.
 *
 * The FSM is intentionally hardware-independent. It does not read pins,
 * configure peripherals, or write to LEDs; those duties belong to the
 * driver layer (Button, Led) and to the application that wires them
 * together. This separation makes the FSM straightforward to unit-test
 * and reusable across different output devices (digital LED, LCD line,
 * relay, serial log, etc.).
 *
 * Example wiring at the application layer:
 * @code
 *   ButtonLedFsm fsm;
 *   fsm.init();
 *   if (button.wasPressed()) {
 *       fsm.processEvent();
 *       led.set(fsm.getOutput());
 *   }
 * @endcode
 */

#ifndef BUTTON_LED_FSM_H
#define BUTTON_LED_FSM_H

#include <stdint.h>

/**
 * @enum ButtonLedState
 * @brief Enumerated state identifiers for the Button-LED automaton.
 *
 * The numeric values are stable and used to index the internal state
 * table, so they MUST remain contiguous and start from 0.
 */
enum ButtonLedState {
    LED_OFF_STATE = 0,  ///< Output asserted low: LED is OFF.
    LED_ON_STATE  = 1,  ///< Output asserted high: LED is ON.

    BUTTON_LED_STATE_COUNT
};

/**
 * @class ButtonLedFsm
 * @brief Two-state Moore FSM that toggles an output on each input event.
 *
 * The class stores the current state and exposes a small, focused API
 * mirroring the four-step Moore evaluation skeleton from the lab manual:
 * read output, (optionally) wait, read input, compute next state.
 */
class ButtonLedFsm {
public:
    /**
     * @brief Construct the FSM. The initial state is LED_OFF_STATE.
     */
    ButtonLedFsm();

    /**
     * @brief Reset the FSM to its initial state (LED_OFF_STATE).
     *
     * Should be called once during application setup. Calling it again
     * is safe and forces the automaton back to the OFF state.
     */
    void init();

    /**
     * @brief Process a single confirmed press event.
     *
     * Drives the state transition function: looks up the next state in
     * the FSM table for the current state with input "1" (button pressed)
     * and commits the transition. Any subsequent call to @ref getOutput()
     * or @ref getState() reflects the new state.
     */
    void processEvent();

    /**
     * @brief Get the current state of the automaton.
     * @return The active @ref ButtonLedState.
     */
    ButtonLedState getState() const;

    /**
     * @brief Get the Moore output associated with the current state.
     *
     * Because this is a Moore machine, the output is a pure function of
     * the state. By convention 0 means "LED OFF" and 1 means "LED ON",
     * matching the table presented in the lab manual (Tabelul 7.1).
     *
     * @return 0 if the LED should be off, 1 if it should be on.
     */
    uint8_t getOutput() const;

    /**
     * @brief Human-readable name of the current state ("OFF" / "ON").
     *
     * Useful for STDIO/LCD reporting without leaking the enum into the
     * presentation layer.
     */
    const char *getStateName() const;

    /**
     * @brief Test whether the display/output should be refreshed.
     *
     * The flag is set whenever @ref processEvent() actually changes the
     * state; it is one-shot and is cleared by @ref clearChanged(). This
     * matches the pattern used by other FSM modules in this project
     * (e.g. LockFSM) and lets the application avoid redundant LED/LCD
     * writes when nothing has changed.
     *
     * @return true if the output has changed since the last clearChanged().
     */
    bool changed() const;

    /**
     * @brief Clear the "output changed" flag after the consumer has
     *        refreshed its presentation (LED, LCD, serial log, etc.).
     */
    void clearChanged();

private:
    /**
     * @brief Compact state-table row mirroring Listing 7.3 of the manual.
     *
     * @c output : Moore output (0 = LED OFF, 1 = LED ON).
     * @c next   : next-state lookup, indexed by input value (0 or 1).
     */
    struct StateRow {
        uint8_t        output;
        ButtonLedState next[2];
    };

    /// State transition / output table — single source of truth for the FSM.
    static const StateRow _table[BUTTON_LED_STATE_COUNT];

    ButtonLedState _state;     ///< Current FSM state.
    bool           _changed;   ///< Latched flag: state has changed since last clear.
};

#endif // BUTTON_LED_FSM_H
