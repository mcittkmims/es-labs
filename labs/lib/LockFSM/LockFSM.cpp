/**
 * @file LockFSM.cpp
 * @brief Lock System Finite State Machine Implementation
 *
 * Implements the LockFSM class that manages an electronic lock
 * through a finite state machine. Each keypad press drives
 * state transitions, with the LCD display content updated
 * automatically at each transition.
 *
 * State transitions:
 *   IDLE --[*]--> MENU
 *   MENU --[0]--> LOCK_CONFIRM --[#]--> (lock) --> RESULT
 *   MENU --[1]--> UNLOCK_WAIT_STAR --[*]--> UNLOCK_PWD --[#]--> RESULT
 *   MENU --[2]--> CHANGE_WAIT_STAR --[*]--> CHANGE_OLD_PWD --[*]--> CHANGE_NEW_PWD --[#]--> RESULT
 *   MENU --[3]--> STATUS_CONFIRM --[#]--> RESULT
 *   RESULT --[timeout]--> IDLE
 */

#include "LockFSM.h"
#include <string.h>
#include <stdio.h>

/// Default password set at initialization
static const char DEFAULT_PASSWORD[] = "1234";

// ============================================================
// Constructor and Initialization
// ============================================================

LockFSM::LockFSM()
    : _state(STATE_IDLE)
    , _locked(true)
    , _inputLen(0)
    , _displayChanged(true)
    , _resultStartTime(0)
{
    strncpy(_password, DEFAULT_PASSWORD, MAX_PWD_LEN);
    _password[MAX_PWD_LEN] = '\0';
    _inputBuffer[0] = '\0';
    _oldPwdBuffer[0] = '\0';
    memset(&_display, 0, sizeof(_display));
}

void LockFSM::init() {
    _state = STATE_IDLE;
    _locked = true;
    strncpy(_password, DEFAULT_PASSWORD, MAX_PWD_LEN);
    _password[MAX_PWD_LEN] = '\0';
    clearInput();
    _oldPwdBuffer[0] = '\0';
    updateDisplay();
    printf("[FSM] Initialized. Default password: %s\r\n", DEFAULT_PASSWORD);
}

// ============================================================
// Key Processing (Main FSM Logic)
// ============================================================

void LockFSM::processKey(char key) {
    if (key == 0) return;

    printf("[KEY] '%c' in state %d\r\n", key, (int)_state);

    switch (_state) {

        // --- IDLE: waiting for * to begin ---
        case STATE_IDLE:
            if (key == '*') {
                setState(STATE_MENU);
            }
            // All other keys ignored in IDLE
            break;

        // --- MENU: show options, wait for command digit 0-3 ---
        case STATE_MENU:
            if (key == '0') {
                setState(STATE_LOCK_CONFIRM);
            } else if (key == '1') {
                setState(STATE_UNLOCK_WAIT_STAR);
            } else if (key == '2') {
                setState(STATE_CHANGE_WAIT_STAR);
            } else if (key == '3') {
                setState(STATE_STATUS_CONFIRM);
            } else if (key == '*') {
                // Refresh menu display
                updateDisplay();
            } else {
                setResult("Invalid option!", "Press * to start");
            }
            break;

        // --- CMD 0: Lock --- waiting for # to confirm ---
        case STATE_LOCK_CONFIRM:
            if (key == '#') {
                _locked = true;
                printf("[LOCK] Locked unconditionally\r\n");
                setResult("Lock Activated", "Door is LOCKED");
            } else if (key == '*') {
                setState(STATE_MENU);
            }
            break;

        // --- CMD 1: Unlock --- waiting for * to begin password ---
        case STATE_UNLOCK_WAIT_STAR:
            if (key == '*') {
                clearInput();
                setState(STATE_UNLOCK_PWD);
            } else if (key == '#') {
                setResult("Error: need pwd", "Use *1*pwd#");
            }
            break;

        // --- CMD 1: Unlock --- entering password digits ---
        case STATE_UNLOCK_PWD:
            if (key >= '0' && key <= '9') {
                appendDigit(key);
                updateDisplay();
            } else if (key == '#') {
                // Execute unlock attempt
                if (strcmp(_inputBuffer, _password) == 0) {
                    _locked = false;
                    printf("[LOCK] Unlocked successfully\r\n");
                    setResult("Access Granted!", "Door is OPEN");
                } else {
                    printf("[LOCK] Wrong password entered\r\n");
                    setResult("Wrong Password!", "Access Denied");
                }
                clearInput();
            } else if (key == '*') {
                // Clear and restart password entry
                clearInput();
                updateDisplay();
            }
            break;

        // --- CMD 2: Change --- waiting for * to begin old password ---
        case STATE_CHANGE_WAIT_STAR:
            if (key == '*') {
                clearInput();
                setState(STATE_CHANGE_OLD_PWD);
            } else if (key == '#') {
                setResult("Error: need pwd", "Use *2*old*new#");
            }
            break;

        // --- CMD 2: Change --- entering old password digits ---
        case STATE_CHANGE_OLD_PWD:
            if (key >= '0' && key <= '9') {
                appendDigit(key);
                updateDisplay();
            } else if (key == '*') {
                // Save old password buffer and move to new password
                strncpy(_oldPwdBuffer, _inputBuffer, MAX_PWD_LEN);
                _oldPwdBuffer[MAX_PWD_LEN] = '\0';
                clearInput();
                setState(STATE_CHANGE_NEW_PWD);
            } else if (key == '#') {
                setResult("Error: need new", "Use *2*old*new#");
            }
            break;

        // --- CMD 2: Change --- entering new password digits ---
        case STATE_CHANGE_NEW_PWD:
            if (key >= '0' && key <= '9') {
                appendDigit(key);
                updateDisplay();
            } else if (key == '#') {
                // Verify old password and apply change
                if (strcmp(_oldPwdBuffer, _password) == 0) {
                    if (_inputLen > 0) {
                        strncpy(_password, _inputBuffer, MAX_PWD_LEN);
                        _password[MAX_PWD_LEN] = '\0';
                        printf("[LOCK] Password changed to: %s\r\n",
                               _password);
                        setResult("Pwd Changed!", "Successfully");
                    } else {
                        setResult("Error: empty pw", "Try again");
                    }
                } else {
                    printf("[LOCK] Wrong old password\r\n");
                    setResult("Wrong Old Pwd!", "Change Denied");
                }
                clearInput();
                _oldPwdBuffer[0] = '\0';
            } else if (key == '*') {
                // Clear and restart new password entry
                clearInput();
                updateDisplay();
            }
            break;

        // --- CMD 3: Status --- waiting for # to display ---
        case STATE_STATUS_CONFIRM:
            if (key == '#') {
                if (_locked) {
                    printf("[LOCK] Status: LOCKED\r\n");
                    setResult("Lock Status:", "** LOCKED **");
                } else {
                    printf("[LOCK] Status: UNLOCKED\r\n");
                    setResult("Lock Status:", "** UNLOCKED **");
                }
            } else if (key == '*') {
                setState(STATE_MENU);
            }
            break;

        // --- RESULT: showing message, any key returns ---
        case STATE_SHOW_RESULT:
            if (key == '*') {
                setState(STATE_MENU);
            } else {
                setState(STATE_IDLE);
            }
            break;
    }
}

// ============================================================
// Periodic Update (Timed Transitions)
// ============================================================

void LockFSM::update() {
    if (_state == STATE_SHOW_RESULT) {
        if (millis() - _resultStartTime >= RESULT_DISPLAY_MS) {
            setState(STATE_IDLE);
        }
    }
}

// ============================================================
// Accessors
// ============================================================

LockFSMState LockFSM::getState() const {
    return _state;
}

bool LockFSM::isLocked() const {
    return _locked;
}

const LockDisplay& LockFSM::getDisplay() const {
    return _display;
}

bool LockFSM::displayChanged() const {
    return _displayChanged;
}

void LockFSM::clearDisplayChanged() {
    _displayChanged = false;
}

// ============================================================
// Private Helpers
// ============================================================

void LockFSM::setState(LockFSMState newState) {
    _state = newState;
    printf("[FSM] -> state %d\r\n", (int)_state);
    updateDisplay();
}

void LockFSM::updateDisplay() {
    _displayChanged = true;

    switch (_state) {
        case STATE_IDLE:
            strncpy(_display.line1, "  Smart Lock    ", 17);
            strncpy(_display.line2, "Press * to start", 17);
            break;

        case STATE_MENU:
            strncpy(_display.line1, "0:Lock 1:Unlock ", 17);
            strncpy(_display.line2, "2:ChPwd 3:Status", 17);
            break;

        case STATE_LOCK_CONFIRM:
            strncpy(_display.line1, "CMD: Lock       ", 17);
            strncpy(_display.line2, "Press # to exec ", 17);
            break;

        case STATE_UNLOCK_WAIT_STAR:
            strncpy(_display.line1, "CMD: Unlock     ", 17);
            strncpy(_display.line2, "Press * for pwd ", 17);
            break;

        case STATE_UNLOCK_PWD: {
            strncpy(_display.line1, "Enter password: ", 17);
            // Build masked password display
            char mask[17];
            uint8_t i;
            for (i = 0; i < _inputLen && i < 16; i++) {
                mask[i] = '*';
            }
            // Pad with spaces
            for (; i < 16; i++) {
                mask[i] = ' ';
            }
            mask[16] = '\0';
            strncpy(_display.line2, mask, 17);
            break;
        }

        case STATE_CHANGE_WAIT_STAR:
            strncpy(_display.line1, "CMD: Change Pwd ", 17);
            strncpy(_display.line2, "Press * for pwd ", 17);
            break;

        case STATE_CHANGE_OLD_PWD: {
            strncpy(_display.line1, "Old password:   ", 17);
            char mask[17];
            uint8_t i;
            for (i = 0; i < _inputLen && i < 16; i++) {
                mask[i] = '*';
            }
            for (; i < 16; i++) {
                mask[i] = ' ';
            }
            mask[16] = '\0';
            strncpy(_display.line2, mask, 17);
            break;
        }

        case STATE_CHANGE_NEW_PWD: {
            strncpy(_display.line1, "New password:   ", 17);
            char mask[17];
            uint8_t i;
            for (i = 0; i < _inputLen && i < 16; i++) {
                mask[i] = '*';
            }
            for (; i < 16; i++) {
                mask[i] = ' ';
            }
            mask[16] = '\0';
            strncpy(_display.line2, mask, 17);
            break;
        }

        case STATE_STATUS_CONFIRM:
            strncpy(_display.line1, "CMD: Status     ", 17);
            strncpy(_display.line2, "Press # to exec ", 17);
            break;

        case STATE_SHOW_RESULT:
            // Display content is already set by setResult()
            break;
    }
}

void LockFSM::setResult(const char *line1, const char *line2) {
    _state = STATE_SHOW_RESULT;
    _resultStartTime = millis();
    strncpy(_display.line1, line1, 16);
    _display.line1[16] = '\0';
    strncpy(_display.line2, line2, 16);
    _display.line2[16] = '\0';
    _displayChanged = true;
    printf("[FSM] Result: %s | %s\r\n", _display.line1, _display.line2);
}

void LockFSM::clearInput() {
    _inputBuffer[0] = '\0';
    _inputLen = 0;
}

void LockFSM::appendDigit(char digit) {
    if (_inputLen < MAX_PWD_LEN) {
        _inputBuffer[_inputLen] = digit;
        _inputLen++;
        _inputBuffer[_inputLen] = '\0';
    }
}
