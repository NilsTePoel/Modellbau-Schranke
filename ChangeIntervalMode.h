#pragma once

#include <LiquidCrystal_I2C.h>

#include "Constants.h"

Mode changeAutomaticModeInterval(LiquidCrystal_I2C &lcd, AutomaticModeInterval &interval, bool isNight);