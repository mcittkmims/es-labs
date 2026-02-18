/**
 * @file CommandParser.h
 * @brief Serial Command Parser Interface
 *
 * Provides a reusable command parsing module that interprets
 * text commands received from the serial terminal. Supports
 * extensible command types and case-insensitive matching.
 *
 * Usage:
 *   CommandType cmd = parseCommand("led on");
 *   if (cmd == CMD_LED_ON) { ... }
 */

#ifndef COMMAND_PARSER_H
#define COMMAND_PARSER_H

#include <stdint.h>

/**
 * @enum CommandType
 * @brief Enumeration of recognized serial commands.
 */
enum CommandType {
    CMD_UNKNOWN,   ///< Unrecognized or invalid command
    CMD_LED_ON,    ///< Command to turn the LED ON
    CMD_LED_OFF    ///< Command to turn the LED OFF
};

/**
 * @brief Parse a text string into a CommandType.
 *
 * Trims leading/trailing whitespace and performs
 * case-insensitive matching against known commands.
 *
 * @param input Null-terminated input string from the user.
 * @return The identified CommandType, or CMD_UNKNOWN if not recognized.
 */
CommandType parseCommand(const char *input);

#endif // COMMAND_PARSER_H
