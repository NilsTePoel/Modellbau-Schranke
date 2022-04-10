#include "Debug.h"

void setupDebugPrinting() {
  #ifdef DEBUG_MODE
    Serial.begin(baudRate);
  #endif
}