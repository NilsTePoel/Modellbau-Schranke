#pragma once

#include <Arduino.h>

class TrafficLight {
public:
  TrafficLight(uint8_t redLedPin, uint8_t yellowLedPin, uint8_t greenLedPin);

  void green() const;
  void yellow() const;
  void red() const;
  void redYellow() const;
  void off() const;

private:
  const uint8_t m_redLedPin;
  const uint8_t m_yellowLedPin;
  const uint8_t m_greenLedPin;
};