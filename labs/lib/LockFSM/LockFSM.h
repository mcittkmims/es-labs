/**
 * @file LockFSM.h
 * @brief Lock System Finite State Machine Interface
 *
 * Implements the core logic for a keypad-based electronic lock system
 * using a finite state machine (FSM). The FSM processes keypad input
 * and manages lock/unlock operations, password verification, and
 * password changes.
 *
 * Supported commands (entered via keypad):
 *   *0#          - Lock unconditionally
 *   *1*password# - Unlock with password
 *   *2*old*new#  - Change password (old -> new)
 *   *3#          - Display current lock status
 *
 * The FSM is hardware-independent: it accepts key characters as input
 * and outputs display data (two 16-character lines) for the LCD.
 * LED control and LCD updates are handled by the application layer.
 *
 * Usage:
 *   LockFSM fsm;
 *   fsm.init();
 *   fsm.processKey('*');
 *   if (fsm.displayChanged()) {
 *       lcd.showTwoLines(fsm.getDisplay().line1, fsm.getDisplay().line2);
 *       fsm.clearDisplayChanged();
 *   }
 */

#ifndef LOCK_FSM_H
#define LOCK_FSM_H

#include <Arduino.h>

/// Maximum password length (digits)
static const uint8_t MAX_PWD_LEN = 8;

/// Duration (ms) to show result messages before returning to idle
static const unsigned long RESULT_DISPLAY_MS = 2500;

/**
 * @enum LockFSMState
 * @brief Enumeration of all FSM states.
 */
enum LockFSMState {
    STATE_IDLE,              ///< Waiting for * to begin a command
    STATE_MENU,              ///< Showing menu options, waiting for 0-3
    STATE_LOCK_CONFIRM,      ///< Cmd 0: waiting for # to confirm lock
    STATE_UNLOCK_WAIT_STAR,  ///< Cmd 1: waiting for * to begin password entry
    STATE_UNLOCK_PWD,        ///< Cmd 1: entering password digits, # to execute
    STATE_CHANGE_WAIT_STAR,  ///< Cmd 2: waiting for * to begin old password entry
    STATE_CHANGE_OLD_PWD,    ///< Cmd 2: entering old password digits, * to continue
    STATE_CHANGE_NEW_PWD,    ///< Cmd 2: entering new password digits, # to execute
    STATE_STATUS_CONFIRM,    ///< Cmd 3: waiting for # to show status
    STATE_SHOW_RESULT        ///< Displaying result message (auto-timeout)
};

/**
 * @struct LockDisplay
 * @brief Holds the two display lines for the LCD (16 chars each).
 */
struct LockDisplay {
    char line1[17];  ///< First LCD row (16 chars + null terminator)
    char line2[17];  ///< Second LCD row (16 chars + null terminator)
};

/**
 * @class LockFSM
 * @brief Finite State Machine for the electronic lock system.
 *
 * Manages state transitions, password storage, lock state,
 * and display content. Hardware-independent; the application
 * layer reads display data and controls LEDs/LCD accordingly.
 */
class LockFSM {
public:
    /**
     * @brief Construct a new LockFSM with default password "1234".
     */
    LockFSM();

    /**
     * @brief Initialize (or reset) the FSM to the IDLE state.
     *
     * Sets the lock to LOCKED, password to default "1234",
     * and updates the display for the idle state.
     */
    void init();

    /**
     * @brief Process a single key press from the keypad.
     *
     * Drives the state machine forward based on the pressed key.
     * Valid keys are '0'-'9', '*', '#', 'A'-'D'.
     * Invalid or unexpected keys are ignored or produce error messages.
     *
     * @param key The key character from the keypad (0 if no key).
     */
    void processKey(char key);

    /**
     * @brief Periodic update for timed state transitions.
     *
     * Must be called every loop iteration. Handles the automatic
     * transition from STATE_SHOW_RESULT back to STATE_IDLE after
     * the result display timeout elapses.
     */
    void update();

    /**
     * @brief Get the current FSM state.
     * @return The current LockFSMState.
     */
    LockFSMState getState() const;

    /**
     * @brief Check if the lock is currently engaged.
     * @return True if locked, false if unlocked.
     */
    bool isLocked() const;

    /**
     * @brief Get the current display content.
     * @return Reference to the LockDisplay struct with line1 and line2.
     */
    const LockDisplay& getDisplay() const;

    /**
     * @brief Check if the display content was updated since last clear.
     * @return True if display data changed and needs to be redrawn.
     */
    bool displayChanged() const;

    /**
     * @brief Clear the display-changed flag after the LCD is updated.
     */
    void clearDisplayChanged();

private:
    LockFSMState _state;                   ///< Current FSM state
    bool _locked;                          ///< True if lock is engaged
    char _password[MAX_PWD_LEN + 1];       ///< Current valid password
    char _inputBuffer[MAX_PWD_LEN + 1];    ///< Buffer for password digit entry
    char _oldPwdBuffer[MAX_PWD_LEN + 1];   ///< Buffer for old password (during change)
    uint8_t _inputLen;                     ///< Number of digits in input buffer
    LockDisplay _display;                  ///< Current display content
    bool _displayChanged;                  ///< Flag: display needs LCD update
    unsigned long _resultStartTime;        ///< Timestamp when result was shown

    /**
     * @brief Transition to a new state and update the display.
     * @param newState The target FSM state.
     */
    void setState(LockFSMState newState);

    /**
     * @brief Update the display content based on the current state.
     */
    void updateDisplay();

    /**
     * @brief Set a result message and transition to STATE_SHOW_RESULT.
     * @param line1 First line of the result message (max 16 chars).
     * @param line2 Second line of the result message (max 16 chars).
     */
    void setResult(const char *line1, const char *line2);

    /**
     * @brief Clear the input buffer and reset the digit counter.
     */
    void clearInput();

    /**
     * @brief Append a digit character to the input buffer.
     * @param digit A digit character '0'-'9'.
     */
    void appendDigit(char digit);
};

#endif // LOCK_FSM_H
