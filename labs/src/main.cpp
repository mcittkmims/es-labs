/**
 * @file main.cpp
 * @brief Application Entry Point — Lab Selector
 *
 * This file serves as the central entry point for all laboratory work.
 * It delegates setup() and loop() to the currently selected lab module.
 *
 * To switch between labs, change the #include and function calls below.
 * No code needs to be deleted — simply comment/uncomment the desired lab.
 */

#include <Arduino.h>

// --- Select the active lab by uncommenting the corresponding include ---
#include "lab1_1_main.h"
// #include "lab1_2_main.h"
// #include "lab2_1_main.h"

void setup() {
    // --- Call the setup function of the active lab ---
    lab1_1Setup();
}

void loop() {
    // --- Call the loop function of the active lab ---
    lab1_1Loop();
}