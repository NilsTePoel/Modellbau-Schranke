#pragma once

#include <LiquidCrystal_I2C.h>

#include "src/libraries/TrafficLight.h"
#include "src/libraries/Barrier.h"
#include "Constants.h"

Mode manualMode(LiquidCrystal_I2C &lcd, const TrafficLight &trafficLight, Barrier &barrier);