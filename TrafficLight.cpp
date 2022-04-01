#include <Arduino.h>

#include "TrafficLight.h"

TrafficLight::TrafficLight(uint8_t redLedPin, uint8_t yellowLedPin, uint8_t greenLedPin)
  : m_redLedPin(redLedPin), m_yellowLedPin(yellowLedPin), m_greenLedPin(greenLedPin) {
  pinMode(m_redLedPin, OUTPUT);
  pinMode(m_yellowLedPin, OUTPUT);
  pinMode(m_greenLedPin, OUTPUT);
  off();
}

void TrafficLight::green() {
  digitalWrite(m_redLedPin, LOW);
  digitalWrite(m_yellowLedPin, LOW);
  digitalWrite(m_greenLedPin, HIGH);
}

void TrafficLight::yellow() {
  digitalWrite(m_redLedPin, LOW);
  digitalWrite(m_yellowLedPin, HIGH);
  digitalWrite(m_greenLedPin, LOW);
}

void TrafficLight::red() {
  digitalWrite(m_redLedPin, HIGH);
  digitalWrite(m_yellowLedPin, LOW);
  digitalWrite(m_greenLedPin, LOW);
}

void TrafficLight::redYellow() {
  digitalWrite(m_redLedPin, HIGH);
  digitalWrite(m_yellowLedPin, HIGH);
  digitalWrite(m_greenLedPin, LOW);
}

void TrafficLight::off() {
  digitalWrite(m_redLedPin, LOW);
  digitalWrite(m_yellowLedPin, LOW);
  digitalWrite(m_greenLedPin, LOW);
}