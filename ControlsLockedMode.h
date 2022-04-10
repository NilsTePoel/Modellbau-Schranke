#pragma once

#include <LiquidCrystal_I2C.h>

#include "src/libraries/RFIDReader.h"
#include "Constants.h"

Mode controlsLockedMode(LiquidCrystal_I2C &lcd, RFIDReader &reader);