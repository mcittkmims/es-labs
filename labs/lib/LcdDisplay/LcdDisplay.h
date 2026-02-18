/**
 * @file LcdDisplay.h
 * @brief LCD Display Driver Interface (I2C)
 *
 * Provides a reusable abstraction for a 16x2 LCD display connected
 * via the I2C bus. Wraps the LiquidCrystal_I2C library behind a
 * clean interface for line-based text display operations.
 *
 * Usage:
 *   LcdDisplay lcd(0x27, 16, 2);
 *   lcd.init();
 *   lcd.showTwoLines("Hello", "World");
 */

#ifndef LCD_DISPLAY_H
#define LCD_DISPLAY_H

#include <Arduino.h>
#include <LiquidCrystal_I2C.h>

/**
 * @class LcdDisplay
 * @brief Controls a 16x2 LCD display over I2C.
 *
 * Provides methods for initialization, clearing, cursor positioning,
 * and writing text to individual lines or both lines simultaneously.
 */
class LcdDisplay {
public:
    /**
     * @brief Construct a new LcdDisplay object.
     * @param i2cAddress I2C address of the LCD (typically 0x27).
     * @param cols Number of columns (typically 16).
     * @param rows Number of rows (typically 2).
     */
    LcdDisplay(uint8_t i2cAddress, uint8_t cols, uint8_t rows);

    /**
     * @brief Initialize the LCD hardware, enable backlight, clear screen.
     */
    void init();

    /**
     * @brief Clear the entire display.
     */
    void clear();

    /**
     * @brief Set the cursor position.
     * @param col Column (0-based).
     * @param row Row (0-based).
     */
    void setCursor(uint8_t col, uint8_t row);

    /**
     * @brief Print a text string at the current cursor position.
     * @param text Null-terminated string to display.
     */
    void print(const char *text);

    /**
     * @brief Write text to a specific row, padding with spaces.
     *
     * Clears the entire row by padding the text to the full
     * column width, eliminating leftover characters from
     * previous content.
     *
     * @param row Row number (0 or 1).
     * @param text Null-terminated string to display.
     */
    void printLine(uint8_t row, const char *text);

    /**
     * @brief Display text on both lines simultaneously.
     * @param line1 Text for the first row.
     * @param line2 Text for the second row.
     */
    void showTwoLines(const char *line1, const char *line2);

    /**
     * @brief Enable or disable the LCD backlight.
     * @param on True to enable, false to disable.
     */
    void backlight(bool on);

private:
    LiquidCrystal_I2C _lcd;  ///< Underlying I2C LCD driver
    uint8_t _cols;            ///< Number of display columns
    uint8_t _rows;            ///< Number of display rows
};

#endif // LCD_DISPLAY_H
