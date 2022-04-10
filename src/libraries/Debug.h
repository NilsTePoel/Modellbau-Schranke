#pragma once

#include <Arduino.h>

// Aktiviert zusätzliche Ausgaben in der Konsole für die Fehlersuche
#define DEBUG_MODE

#ifdef DEBUG_MODE
  #define DEBUG_PRINT(...) Serial.print(__VA_ARGS__)
  #define DEBUG_PRINTLN(...) Serial.println(__VA_ARGS__)
#else
  #define DEBUG_PRINT(...)
  #define DEBUG_PRINTLN(...)
#endif

const uint16_t baudRate = 9600;

void setupDebugPrinting();
