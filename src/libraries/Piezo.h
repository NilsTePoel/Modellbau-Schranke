#pragma once

#include <Arduino.h>

class Piezo {
public:
  Piezo(uint8_t piezoPin, uint8_t ledPin);

  void enable();
  void disable();

  void alarmOn() const;
  void alarmOff() const;

private:
  const uint8_t m_piezoPin;
  const uint8_t m_ledPin;
  bool m_enabled;
};