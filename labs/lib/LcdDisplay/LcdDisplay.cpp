/**
 * @file LcdDisplay.cpp
 * @brief LCD Display Driver Implementation (I2C)
 *
 * Implements the LcdDisplay class methods using the LiquidCrystal_I2C
 * library for I2C-based 16x2 LCD communication. Provides line-level
 * text operations with automatic space padding to prevent display artifacts.
 */

#include "LcdDisplay.h"
#include <string.h>

LcdDisplay::LcdDisplay(uint8_t i2cAddress, uint8_t cols, uint8_t rows)
    : _lcd(i2cAddress, cols, rows), _cols(cols), _rows(rows) {}

void LcdDisplay::init() {
    _lcd.init();
    _lcd.backlight();
    _lcd.clear();
}

void LcdDisplay::clear() {
    _lcd.clear();
}

void LcdDisplay::setCursor(uint8_t col, uint8_t row) {
    _lcd.setCursor(col, row);
}

void LcdDisplay::print(const char *text) {
    _lcd.print(text);
}

void LcdDisplay::printLine(uint8_t row, const char *text) {
    _lcd.setCursor(0, row);

    // Write text characters up to column limit
    uint8_t len = strlen(text);
    if (len > _cols) {
        len = _cols;
    }
    for (uint8_t i = 0; i < len; i++) {
        _lcd.write(text[i]);
    }

    // Pad remaining columns with spaces to clear old content
    for (uint8_t i = len; i < _cols; i++) {
        _lcd.write(' ');
    }
}

void LcdDisplay::showTwoLines(const char *line1, const char *line2) {
    printLine(0, line1);
    printLine(1, line2);
}

void LcdDisplay::backlight(bool on) {
    if (on) {
        _lcd.backlight();
    } else {
        _lcd.noBacklight();
    }
}
