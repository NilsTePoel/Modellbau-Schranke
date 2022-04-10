#pragma once

#include <LiquidCrystal_I2C.h>

#include "src/libraries/TrafficLight.h"
#include "src/libraries/Barrier.h"
#include "src/libraries/RFIDReader.h"
#include "Constants.h"

Mode automaticMode(LiquidCrystal_I2C &lcd, const TrafficLight &trafficLight, Barrier &barrier, RFIDReader &reader, AutomaticModeInterval &interval);