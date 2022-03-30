#include <Arduino.h>

#include "DistanceSensor.h"

DistanceSensor::DistanceSensor(uint8_t triggerPin, uint8_t echoPin, uint8_t minimumDistance) 
  : m_triggerPin(triggerPin), m_echoPin(echoPin), m_minimumDistance(minimumDistance) {
  pinMode(m_triggerPin, OUTPUT);
  pinMode(m_echoPin, INPUT);
}

uint32_t DistanceSensor::getDistanceToObstacle() const {
  digitalWrite(m_triggerPin, LOW);
  delay(5);
  digitalWrite(m_triggerPin, HIGH);
  delay(10);
  digitalWrite(m_triggerPin, LOW);

  uint32_t duration = pulseIn(m_echoPin, HIGH); // Dauer, bis der Schall zum Ultraschallsensor zurückkehrt
  uint32_t distance = 0.03432 * (duration / 2); // s = v * t (in cm)

  return distance;
}

bool DistanceSensor::isBlockedByObstacle() const {
  return getDistanceToObstacle() < m_minimumDistance;
}