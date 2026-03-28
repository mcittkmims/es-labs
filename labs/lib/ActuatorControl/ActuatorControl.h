/**
 * @file ActuatorControl.h
 * @brief Binary Actuator Controller Interface
 *
 * Controls a binary actuator (LED or relay) on a digital GPIO pin.
 * Incoming ON/OFF commands are fed through a BinaryConditioner pipeline
 * (saturation → debounce → persistent validation) before the hardware
 * output is updated, filtering out spurious or transient commands.
 *
 * The conditioned actuator state is updated by calling update() at the
 * configured recurrence rate (50–100 ms). The current validated state
 * is exposed via getState() for monitoring by other system components.
 *
 * Architecture:
 *   setRawCommand() ─→ BinaryConditioner::process() ─→ Led (GPIO)
 *
 * Usage:
 *   ActuatorControl act(PIN_ACT, 100, 3);   // 100 ms debounce, 3 confirms
 *   act.init();
 *   act.setRawCommand(ActuatorControl::State::ON);
 *   act.update();   // call every 50–100 ms
 *   if (act.getState() == ActuatorControl::State::ON) { ... }
 */

#ifndef ACTUATOR_CONTROL_H
#define ACTUATOR_CONTROL_H

#include <Arduino.h>
#include "BinaryConditioner.h"
#include "Led.h"

/**
 * @class ActuatorControl
 * @brief Binary actuator with integrated signal conditioning pipeline.
 */
class ActuatorControl {
public:
    /** @brief Possible actuator states. */
    enum class State { OFF, ON };

    /**
     * @brief Construct a new ActuatorControl.
     * @param pin              GPIO pin driving the actuator output.
     * @param debouncePeriodMs Debounce window for command conditioning (ms).
     * @param persistCount     Required consecutive confirmations.
     */
    ActuatorControl(uint8_t pin,
                    uint32_t debouncePeriodMs = 100,
                    uint8_t persistCount = 3);

    /**
     * @brief Initialize the GPIO pin as OUTPUT and set the actuator to OFF.
     */
    void init();

    /**
     * @brief Submit a raw command from the user input layer.
     *
     * The command is stored and applied on the next update() call.
     * Multiple calls between update() ticks are coalesced to the last value.
     *
     * @param cmd Desired actuator state (ON or OFF).
     */
    void setRawCommand(State cmd);

    /**
     * @brief Run one conditioning step and update the hardware output.
     *
     * Must be called at the configured recurrence rate (50–100 ms).
     * Passes the latest raw command through BinaryConditioner, then drives
     * the GPIO output to match the conditioned result.
     */
    void update();

    /**
     * @brief Get the current conditioned (hardware-applied) actuator state.
     * @return Validated actuator state.
     */
    State getState() const;

    /**
     * @brief Get the last raw (unconditioned) command submitted.
     * @return Last commanded state before conditioning.
     */
    State getRawCommand() const;

    /**
     * @brief Check whether signal conditioning is currently in progress.
     * @return true if a transition is being validated.
     */
    bool isDebouncing() const;

    /**
     * @brief Get the current consecutive confirmation count.
     * @return Confirmations accumulated for the pending transition.
     */
    uint8_t getConfirmCount() const;

    /**
     * @brief Get the required confirmation count to commit a transition.
     */
    uint8_t getRequiredConfirmCount() const;

private:
    Led               _led;
    BinaryConditioner _conditioner;
    State             _rawCommand;
    State             _currentState;
};

#endif // ACTUATOR_CONTROL_H
