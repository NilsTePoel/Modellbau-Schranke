#pragma once
class TrafficLight {
public:
  TrafficLight(uint8_t redLedPin, uint8_t yellowLedPin, uint8_t greenLedPin);

  void green();
  void yellow();
  void red();
  void redYellow();
  void off();

private:
  const uint8_t m_redLedPin;
  const uint8_t m_yellowLedPin;
  const uint8_t m_greenLedPin;
};