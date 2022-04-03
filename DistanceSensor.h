#pragma once

#include <Arduino.h>

class DistanceSensor {
public:
  DistanceSensor(uint8_t triggerPin, uint8_t echoPin, uint8_t minimumDistance);

  bool isBlockedByObstacle() const;

private:
  const uint8_t m_triggerPin;
  const uint8_t m_echoPin;
  const uint8_t m_minimumDistance;

  uint32_t getDistanceToObstacle() const;
};