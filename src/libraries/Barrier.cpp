#include "Barrier.h"

Barrier::Barrier(uint16_t spu, const uint8_t *stepperPins, uint8_t stepperSpeed, LiquidCrystal_I2C &lcd,
    const TrafficLight &trafficLight, const Piezo &piezo, const DistanceSensor &distanceSensor)
  : m_spu(spu), m_stepperPins(stepperPins), m_motor(m_spu, m_stepperPins[0], m_stepperPins[1], m_stepperPins[2], m_stepperPins[3]),
    m_lcd(lcd), m_trafficLight(trafficLight), m_piezo(piezo), m_distanceSensor(distanceSensor) {
  m_motor.setSpeed(stepperSpeed);
}

void Barrier::step(int16_t numSteps) {
  m_motor.step(numSteps);
  disableStepperMotor();
}

void Barrier::close() {
  int16_t numSteps = m_spu / 4;
  step(-numSteps);
}

void Barrier::open() {
  int16_t numSteps = m_spu / 4;
  step(numSteps);
}

void Barrier::closeAndOpen() {
  m_lcd.setCursor(0, 0);
  m_lcd.print("Schranke        ");
  m_lcd.setCursor(0, 1);
  m_lcd.print("schlie\342t sich!  ");

  // Ampel schaltet erst auf gelb und dann auf rot um
  m_trafficLight.yellow();
  m_piezo.alarmOn();
  delay(500);
  m_piezo.alarmOff();
  delay(trafficLightYellowDelay - 500);
  m_trafficLight.red();

  // Die Schranke soll sich nicht schließen, wenn sich ein Objekt unter ihr befindet
  while (m_distanceSensor.isBlockedByObstacle()) {
    m_lcd.setCursor(0, 1);
    m_lcd.print("blockiert!    ");
    delay(1000);
  }

  m_lcd.setCursor(0, 1);
  m_lcd.print("schlie\342t sich!");

  close();

  m_lcd.setCursor(0, 1);
  m_lcd.print("geschlossen   ");

  delay(barrierClosedDelay);

  m_lcd.setCursor(0, 1);
  m_lcd.print("\357ffnet sich!");

  open();

  // Ampel schaltet erst auf rot-gelb, dann auf grün
  m_trafficLight.redYellow();
  delay(trafficLightRedYellowDelay);
  m_trafficLight.green();
}

void Barrier::disableStepperMotor() const {
  for (uint8_t i = 0; i < numStepperPins; i++) {
    digitalWrite(m_stepperPins[i], LOW);
  }
}