#include "Barrier.h"

Barrier::Barrier(uint16_t spu, const uint8_t *stepperPins, uint8_t stepperSpeed)
  : m_spu(spu), m_stepperPins(stepperPins), m_motor(m_spu, m_stepperPins[0], m_stepperPins[1], m_stepperPins[2], m_stepperPins[3]) {
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

void Barrier::closeAndOpen(LiquidCrystal_I2C &lcd, const TrafficLight &trafficLight,
  const Piezo &piezo, const DistanceSensor &distanceSensor) {
  lcd.setCursor(0, 0);
  lcd.print("Schranke        ");
  lcd.setCursor(0, 1);
  lcd.print("schlie\342t sich!  ");

  // Ampel schaltet erst auf gelb und dann auf rot um
  trafficLight.yellow();
  piezo.alarmOn();
  delay(500);
  piezo.alarmOff();
  delay(trafficLightYellowDelay - 500);
  trafficLight.red();

  // Die Schranke soll sich nicht schließen, wenn sich ein Objekt unter ihr befindet
  while (distanceSensor.isBlockedByObstacle()) {
    lcd.setCursor(0, 1);
    lcd.print("blockiert!    ");
    delay(1000);
  }

  lcd.setCursor(0, 1);
  lcd.print("schlie\342t sich!");

  close();

  lcd.setCursor(0, 1);
  lcd.print("geschlossen   ");

  delay(barrierClosedDelay);

  lcd.setCursor(0, 1);
  lcd.print("\357ffnet sich!");

  open();

  // Ampel schaltet erst auf rot-gelb, dann auf grün
  trafficLight.redYellow();
  delay(trafficLightRedYellowDelay);
  trafficLight.green();
}

void Barrier::disableStepperMotor() const {
  for (uint8_t i = 0; i < numStepperPins; i++) {
    digitalWrite(m_stepperPins[i], LOW);
  }
}