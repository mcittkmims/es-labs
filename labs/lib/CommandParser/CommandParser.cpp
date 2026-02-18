/**
 * @file CommandParser.cpp
 * @brief Serial Command Parser Implementation
 *
 * Implements command parsing logic with:
 * - Whitespace trimming (leading and trailing)
 * - Case-insensitive comparison
 * - Extensible command matching
 */

#include "CommandParser.h"
#include <string.h>
#include <ctype.h>

/// Maximum length for internal string buffers during parsing.
static const uint8_t PARSE_BUFFER_SIZE = 64;

/**
 * @brief Convert a string to lowercase (in-place copy).
 *
 * Copies src into dest, converting each character to lowercase.
 * The result is null-terminated and limited to maxLen - 1 characters.
 *
 * @param dest   Destination buffer.
 * @param src    Source string to convert.
 * @param maxLen Maximum size of the destination buffer.
 */
static void toLowerStr(char *dest, const char *src, size_t maxLen) {
    size_t i = 0;
    while (src[i] != '\0' && i < maxLen - 1) {
        dest[i] = tolower((unsigned char)src[i]);
        i++;
    }
    dest[i] = '\0';
}

/**
 * @brief Trim leading and trailing whitespace from a string.
 *
 * Copies the trimmed content of src into dest, stripping all
 * leading and trailing whitespace characters (spaces, tabs,
 * newlines, carriage returns).
 *
 * @param dest   Destination buffer for the trimmed string.
 * @param src    Source string to trim.
 * @param maxLen Maximum size of the destination buffer.
 */
static void trimStr(char *dest, const char *src, size_t maxLen) {
    // Skip leading whitespace
    while (*src && isspace((unsigned char)*src)) {
        src++;
    }

    // Find the end of meaningful content
    size_t len = strlen(src);
    while (len > 0 && isspace((unsigned char)src[len - 1])) {
        len--;
    }

    // Copy trimmed content
    if (len >= maxLen) {
        len = maxLen - 1;
    }
    strncpy(dest, src, len);
    dest[len] = '\0';
}

CommandType parseCommand(const char *input) {
    char trimmed[PARSE_BUFFER_SIZE];
    char lower[PARSE_BUFFER_SIZE];

    // Step 1: Remove leading/trailing whitespace
    trimStr(trimmed, input, sizeof(trimmed));

    // Step 2: Convert to lowercase for case-insensitive matching
    toLowerStr(lower, trimmed, sizeof(lower));

    // Step 3: Match against known commands
    if (strcmp(lower, "led on") == 0) {
        return CMD_LED_ON;
    } else if (strcmp(lower, "led off") == 0) {
        return CMD_LED_OFF;
    }

    return CMD_UNKNOWN;
}
