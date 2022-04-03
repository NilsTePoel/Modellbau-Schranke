#include "Piezo.h"

Piezo::Piezo(uint8_t piezoPin, uint8_t ledPin)
  : m_piezoPin(piezoPin), m_ledPin(ledPin), m_enabled(false) {
  pinMode(m_piezoPin, OUTPUT);
  pinMode(m_ledPin, OUTPUT); // LED zeigt Piezo-Status an
}

void Piezo::enable() {
  m_enabled = true;
  digitalWrite(m_ledPin, HIGH);
}

void Piezo::disable() {
  m_enabled = false;
  digitalWrite(m_ledPin, LOW);
}

void Piezo::alarmOn() {
  if (m_enabled) {
    digitalWrite(m_piezoPin, HIGH);
  }
}

void Piezo::alarmOff() {
  if (m_enabled) {
    digitalWrite(m_piezoPin, LOW);
  }
}