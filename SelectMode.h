#pragma once

#include <Arduino.h>
#include <LiquidCrystal_I2C.h>

#include "src/libraries/Piezo.h"
#include "Constants.h"

Mode selectMode(LiquidCrystal_I2C &lcd, Piezo &piezo);

void printDigit(LiquidCrystal_I2C &lcd, uint8_t digit);