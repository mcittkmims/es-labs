/**
 * @file main.cpp
 * @brief Application Entry Point — Lab Selector
 *
 * This file serves as the central entry point for all laboratory work.
 * It delegates setup() and loop() to the currently selected lab module.
 *
 * The active lab is selected automatically via preprocessor defines
 * set in platformio.ini (e.g., -DLAB1_1, -DLAB1_2). Each PlatformIO
 * environment builds and runs exactly one lab without code changes.
 */

#include <Arduino.h>

// --- Automatically select the active lab based on build flags ---
#if defined(LAB1_1)
    #include "lab1_1_main.h"
#elif defined(LAB1_2)
    #include "lab1_2_main.h"
#else
    #error "No lab selected! Add -DLAB1_x to build_flags in platformio.ini"
#endif

void setup() {
#if defined(LAB1_1)
    lab1_1Setup();
#elif defined(LAB1_2)
    lab1_2Setup();
#endif
}

void loop() {
#if defined(LAB1_1)
    lab1_1Loop();
#elif defined(LAB1_2)
    lab1_2Loop();
#endif
}