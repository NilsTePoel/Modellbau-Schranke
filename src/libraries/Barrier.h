#pragma once

#include <Arduino.h>
#include <Stepper.h>
#include <LiquidCrystal_I2C.h>

#include "TrafficLight.h"
#include "Piezo.h"
#include "DistanceSensor.h"

const uint8_t numStepperPins = 4;

const uint32_t trafficLightYellowDelay = 3000;
const uint32_t barrierClosedDelay = 5000;
const uint32_t trafficLightRedYellowDelay = 3000;

class Barrier {
public:
  Barrier(uint16_t spu, const uint8_t *stepperPins, uint8_t stepperSpeed, LiquidCrystal_I2C &lcd,
    const TrafficLight &trafficLight, const Piezo &piezo, const DistanceSensor &distanceSensor);

  void step(int16_t numSteps);

  void close();
  void open();

  void closeAndOpen();

private:
  const uint16_t m_spu;
  const uint8_t *m_stepperPins;
  Stepper m_motor;
  LiquidCrystal_I2C &m_lcd;
  const TrafficLight &m_trafficLight;
  const Piezo &m_piezo;
  const DistanceSensor &m_distanceSensor;

  void disableStepperMotor() const;
};