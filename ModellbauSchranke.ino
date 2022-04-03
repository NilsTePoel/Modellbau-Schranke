#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <IRremote.h>
#include <Stepper.h>
#include <SPI.h>
#include <MFRC522.h>
#include <TimeLib.h>
#include <DS1307RTC.h>

#include "Constants.h"
#include "Debug.h"
#include "TrafficLight.h"
#include "Piezo.h"
#include "DistanceSensor.h"

LiquidCrystal_I2C lcd(lcdAddress, lcdNumColumns, lcdNumRows);
Stepper motor(spu, stepperPins[0], stepperPins[1], stepperPins[2], stepperPins[3]);
TrafficLight trafficLight(redLedPin, yellowLedPin, greenLedPin);
Piezo piezo(piezoPin, piezoLedPin);
DistanceSensor distanceSensor(triggerPin, echoPin, minimumDistance);
MFRC522 mfrc522(sdaPin, rstPin);

enum class Mode {
  CONTROLS_LOCKED,
  SELECT_MODE,
  MANUAL,
  AUTOMATIC,
  CHANGE_AUTOMATIC_MODE_INTERVAL_DAY,
  CHANGE_AUTOMATIC_MODE_INTERVAL_NIGHT
};

Mode mode = Mode::CONTROLS_LOCKED;

uint16_t automaticModeIntervalDay = 30; // Anzahl der Sekunden, in der die Schranke tagsüber geöffnet ist
uint16_t automaticModeIntervalNight = 60; // Anzahl der Sekunden, in der die Schranke nachts (zw. 18 und 6 Uhr) geöffnet ist
bool automaticModeControlsLocked = false;
bool showTime = false;

void setup() {
  // Debug-Ausgaben
  setupDebugPrinting();

  // LCD
  lcd.init();
  lcd.backlight();
  
  // IR-Empfänger
  IrReceiver.begin(receiverPin, DISABLE_LED_FEEDBACK);

  // Schrittmotor-Geschwindigkeit
  motor.setSpeed(stepperSpeed);

  // RFID-Kit
  SPI.begin();
  mfrc522.PCD_Init();

  // RTC
  setSyncProvider(RTC.get);
  
  // Uhrzeit ausgeben, wenn der Debug-Modus aktiviert ist
  time_t t = now();
  DEBUG_PRINT(hour(t)); DEBUG_PRINT(":");
  DEBUG_PRINT(minute(t)); DEBUG_PRINT(":");
  DEBUG_PRINTLN(second(t));
}

void loop() {
  switch (mode) {
    case Mode::CONTROLS_LOCKED:
      controlsLocked();
      break;
    case Mode::SELECT_MODE:
      selectMode();
      break;
    case Mode::MANUAL:
      manualMode();
      break;
    case Mode::AUTOMATIC:
      automaticMode();
      break;
    case Mode::CHANGE_AUTOMATIC_MODE_INTERVAL_DAY:
      changeAutomaticModeInterval(false);
      break;
    case Mode::CHANGE_AUTOMATIC_MODE_INTERVAL_NIGHT:
      changeAutomaticModeInterval(true);
      break;
  }
}

void controlsLocked() {
  lcd.setCursor(0, 0);
  lcd.print("Steuerung");
  lcd.setCursor(0, 1);
  lcd.print("gesperrt");

  // Warten, bis die Steuerung per RFID-Chip entsperrt wurde
  if (validRFIDTagPresent()) {
    mode = Mode::SELECT_MODE;
  }
}

void selectMode() {
  lcd.setCursor(0, 0);
  lcd.print("Modus w\341hlen    ");
  lcd.setCursor(0, 1);
  if (showTime) {
    time_t t = now();
    printDigit(hour(t)); lcd.print(":");
    printDigit(minute(t)); lcd.print(":");
    printDigit(second(t)); lcd.print("        ");
  } else {
    lcd.print("             ");
  }

  // Wurden Daten empfangen?
  if (IrReceiver.decode()) {
    DEBUG_PRINTLN(IrReceiver.decodedIRData.decodedRawData, DEC);
    IrReceiver.resume(); // Nächsten Wert einlesen

    switch (IrReceiver.decodedIRData.decodedRawData) {
      case buttonA:
        mode = Mode::AUTOMATIC;
        break;
      case buttonB:
        mode = Mode::MANUAL;
        break;
      case buttonX:
        mode = Mode::CHANGE_AUTOMATIC_MODE_INTERVAL_DAY;
        break;
      case buttonLeft:
        piezo.disable();
        break;
      case buttonRight:
        piezo.enable();
        break;
      case buttonDown:
        showTime = !showTime;
        break;
    }
  }
}

void manualMode() {
  trafficLight.green();

  lcd.setCursor(0, 0);
  lcd.print("Manuell         ");
  lcd.setCursor(0, 1);
  lcd.print("                ");
  
  // Wurden Daten empfangen?
  if (IrReceiver.decode()) {
    DEBUG_PRINTLN(IrReceiver.decodedIRData.decodedRawData, DEC);
    IrReceiver.resume(); // Nächsten Wert einlesen

    switch (IrReceiver.decodedIRData.decodedRawData) {
      case buttonLeft:
        lcd.setCursor(0, 0);
        lcd.print("Schranke");
        lcd.setCursor(0, 1);
        lcd.print("schlie\342t sich!");
        motor.step(-512);
        disableMotor();
        break;
      case buttonRight:
        lcd.setCursor(0, 0);
        lcd.print("Schranke");
        lcd.setCursor(0, 1);
        lcd.print("\357ffnet sich");
        motor.step(512);
        disableMotor();
        break;
      case buttonA:
        lcd.setCursor(0, 0);
        lcd.print("100 Schritte");
        lcd.setCursor(0, 1);
        lcd.print("nach links");
        motor.step(-100);
        disableMotor();
        break;
      case buttonB:
        lcd.setCursor(0, 0);
        lcd.print("100 Schritte");
        lcd.setCursor(0, 1);
        lcd.print("nach rechts");
        motor.step(100);
        disableMotor();
        break;
      case buttonDown:
        closeAndOpenBarrier();
        break;
      case buttonUp:
        mode = Mode::SELECT_MODE; // Zurück zum "Modus wählen"-Menü
        trafficLight.off();
        break;
    }
  }
}

void automaticMode() {
  trafficLight.green();

  lcd.setCursor(0, 0);
  lcd.print("Automatisch  ");

  uint16_t interval = (hour() >= 18 || hour() <= 6) ? automaticModeIntervalNight : automaticModeIntervalDay; // Nachts anderes Intervall
  bool backToMenu = false;
  
  for (uint16_t remainingTime = interval; remainingTime > 0; remainingTime--) {
    lcd.setCursor(0, 1);
    lcd.print("Noch ");
    lcd.print(remainingTime);
    lcd.print(" Sek. ");

    if (automaticModeControlsLocked && validRFIDTagPresent()) {
      automaticModeControlsLocked = false;
    } else if (!automaticModeControlsLocked) {
      // Wurden Daten empfangen?
      if (IrReceiver.decode()) {
        DEBUG_PRINTLN(IrReceiver.decodedIRData.decodedRawData, DEC);
        IrReceiver.resume(); // Nächsten Wert einlesen
    
        if (IrReceiver.decodedIRData.decodedRawData == buttonUp) {
          mode = Mode::SELECT_MODE; // Zurück zum "Modus wählen"-Menü
          trafficLight.off();
          backToMenu = true;
          break;
        } else if (IrReceiver.decodedIRData.decodedRawData == buttonX) {
          automaticModeControlsLocked = true;
        }
      }
    }

    delay(1000);
  }
  if (!backToMenu) closeAndOpenBarrier();
}

void changeAutomaticModeInterval(boolean isNight) {
  uint16_t interval = isNight ? automaticModeIntervalNight : automaticModeIntervalDay;
  
  lcd.setCursor(0, 0);
  lcd.print(isNight ? "Interv. (Nacht):" : "Interv. (Tag):  ");
  lcd.setCursor(0, 1);
  lcd.print("< ");
  lcd.print(interval);
  lcd.print(" >  ");

  // Wurden Daten empfangen?
  if (IrReceiver.decode()) {
    DEBUG_PRINTLN(IrReceiver.decodedIRData.decodedRawData, DEC);
    IrReceiver.resume(); // Nächsten Wert einlesen

    switch (IrReceiver.decodedIRData.decodedRawData) {
      case buttonLeft:
        interval = constrain(interval - 1, 1, 999);
        break;
      case buttonRight:
        interval = constrain(interval + 1, 1, 999);
        break;
      case buttonA:
        interval = constrain(interval - 10, 1, 999);
        break;
      case buttonB:
        interval = constrain(interval + 10, 1, 999);
        break;
      case buttonX:
        mode = isNight ? Mode::SELECT_MODE : Mode::CHANGE_AUTOMATIC_MODE_INTERVAL_NIGHT; // Weiter zum nächsten Menü
        break;
      case buttonUp:
        mode = isNight ? Mode::CHANGE_AUTOMATIC_MODE_INTERVAL_DAY : Mode::SELECT_MODE; // Zurück zum letzten Menü
        break;
    }
  }

  if (isNight) automaticModeIntervalNight = interval;
  else automaticModeIntervalDay = interval;
}

void closeAndOpenBarrier() {
  lcd.setCursor(0, 0);
  lcd.print("Schranke        ");
  lcd.setCursor(0, 1);
  lcd.print("schlie\342t sich!  ");

  // Ampel schaltet erst auf gelb und dann auf rot um
  trafficLight.yellow();
  piezo.alarmOn();
  delay(500);
  piezo.alarmOff();
  delay(2500);
  trafficLight.red();
  
  // Die Schranke soll sich nicht schließen, wenn sich ein Objekt unter ihr befindet
  while (distanceSensor.isBlockedByObstacle()) {
    lcd.setCursor(0, 1);
    lcd.print("blockiert!    ");
    delay(1000);
  }

  lcd.setCursor(0, 1);
  lcd.print("schlie\342t sich!");
  
  motor.step(-512); // Schranke wird geschlossen
  disableMotor();  

  lcd.setCursor(0, 1);
  lcd.print("geschlossen   ");
  
  delay(5000);

  lcd.setCursor(0, 1);
  lcd.print("\357ffnet sich!");
  
  motor.step(512); // Schranke wird wieder geöffnet
  disableMotor();

  // Ampel schaltet erst auf rot-gelb, dann auf grün
  trafficLight.redYellow();
  delay(3000);
  trafficLight.green();
}

void disableMotor() {
  for (uint8_t i = 0; i < 4; i++) {
    digitalWrite(stepperPins[i], LOW);
  }
}

bool validRFIDTagPresent() {
  if (mfrc522.PICC_IsNewCardPresent() && mfrc522.PICC_ReadCardSerial()) {
    #ifdef DEBUG_MODE
      mfrc522.PICC_DumpDetailsToSerial(&mfrc522.uid);
    #endif

    // Steuerung entsperren, falls die ID des RFID-Tags einer der eingespeicherten IDs entspricht
    if (mfrc522.uid.size == 4) {
      for (uint8_t i = 0; i < sizeof(validIDs); i++) {
        bool validID = true;
        for (uint8_t j = 0; j < 4; j++) {
          if (mfrc522.uid.uidByte[j] != validIDs[i][j]) validID = false;
        }

        if (validID) {
          return true;
        }
      }
    }
  }

  return false;
}

void printDigit(uint8_t digit) {
  if (digit < 10) lcd.print("0");
  lcd.print(digit);
}
