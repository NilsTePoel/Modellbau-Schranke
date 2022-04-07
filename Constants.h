#pragma once

#include <Arduino.h>

// LCD
const uint8_t lcdAddress = 0x27;
const uint8_t lcdNumColumns = 16;
const uint8_t lcdNumRows = 2;

// Infrarot-Receiver
const uint8_t receiverPin = 11;
const uint32_t buttonA = 0xBA45FF00;
const uint32_t buttonB = 0xB847FF00;
const uint32_t buttonX = 0xBF40FF00;
const uint32_t buttonLeft = 0xBB44FF00;
const uint32_t buttonRight = 0xBC43FF00;
const uint32_t buttonDown = 0xEA15FF00;
const uint32_t buttonUp = 0xB946FF00;

// Schrittmotor
const uint16_t spu = 2048; // Schritte pro Umdrehung
const uint8_t stepperSpeed = 5; // Motor-Geschwindigkeit in Umdrehungen pro Minute
const uint8_t stepperPins[4] = {3, 5, 4, 6};

// Ampel
const uint8_t redLedPin = 28;
const uint8_t yellowLedPin = 24;
const uint8_t greenLedPin = 26;

// Piezo
const uint8_t piezoPin = 8;
const uint8_t piezoLedPin = LED_BUILTIN;

// Ultraschallsensor
const uint8_t triggerPin = 10;
const uint8_t echoPin = A0;
const uint8_t minimumDistance = 5; // Minimale Entfernung zur Schranke in Zentimetern

// RFID Kit
const uint8_t sdaPin = 53;
const uint8_t rstPin = 2;
const uint8_t validIDs[][4] = {{0x3A, 0x26, 0xC5, 0x5C}, {0x87, 0x5B, 0xCF, 0x93}};

enum class Mode {
  CONTROLS_LOCKED,
  SELECT_MODE,
  MANUAL,
  AUTOMATIC,
  CHANGE_AUTOMATIC_MODE_INTERVAL_DAY,
  CHANGE_AUTOMATIC_MODE_INTERVAL_NIGHT
};