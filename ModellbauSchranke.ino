#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <IRremote.h>
#include <TimeLib.h>
#include <DS1307RTC.h>

#include "Constants.h"
#include "Debug.h"
#include "Barrier.h"
#include "TrafficLight.h"
#include "Piezo.h"
#include "DistanceSensor.h"
#include "RFIDReader.h"

LiquidCrystal_I2C lcd(lcdAddress, lcdNumColumns, lcdNumRows);
Barrier barrier(spu, stepperPins, stepperSpeed);
TrafficLight trafficLight(redLedPin, yellowLedPin, greenLedPin);
Piezo piezo(piezoPin, piezoLedPin);
DistanceSensor distanceSensor(triggerPin, echoPin, minimumDistance);
RFIDReader reader(sdaPin, rstPin, validIDs, sizeof(validIDs) / sizeof(validIDs[0]));

Mode mode = Mode::CONTROLS_LOCKED;

bool selectModeShowTime = false; // Soll die aktuelle Uhrzeit im Menü "Modus wählen" angezeigt werden?
uint16_t automaticModeIntervalDay = 30; // Anzahl der Sekunden, in der die Schranke tagsüber geöffnet ist
uint16_t automaticModeIntervalNight = 60; // Anzahl der Sekunden, in der die Schranke nachts (zw. 18 und 6 Uhr) geöffnet ist
bool automaticModeControlsLocked = false; // Ist die Steuerung im automatischen Modus aktuell gesperrt?

void setup() {
  // Debug-Ausgaben
  setupDebugPrinting();

  // LCD
  lcd.init();
  lcd.backlight();
  lcd.clear();

  // IR-Empfänger
  IrReceiver.begin(receiverPin, DISABLE_LED_FEEDBACK);

  // RFID
  reader.begin();

  // RTC
  setSyncProvider(RTC.get);

  // Uhrzeit ausgeben, wenn der Debug-Modus aktiviert ist
  if (timeStatus() == timeSet) {
    DEBUG_PRINTLN("Zeit mit der Real Time Clock synchronisiert.");
  }

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
  if (reader.isValidRFIDTagPresent()) {
    mode = Mode::SELECT_MODE;
  }
}

void selectMode() {
  lcd.setCursor(0, 0);
  lcd.print("Modus w\341hlen    ");
  lcd.setCursor(0, 1);
  if (selectModeShowTime) {
    time_t t = now();
    printDigit(hour(t)); lcd.print(":");
    printDigit(minute(t)); lcd.print(":");
    printDigit(second(t)); lcd.print("        ");
  } else {
    lcd.print("             ");
  }

  // Wurden Daten empfangen?
  if (IrReceiver.decode()) {
    DEBUG_PRINTLN(IrReceiver.decodedIRData.decodedRawData, HEX);
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
        selectModeShowTime = !selectModeShowTime;
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
    DEBUG_PRINTLN(IrReceiver.decodedIRData.decodedRawData, HEX);
    IrReceiver.resume(); // Nächsten Wert einlesen

    switch (IrReceiver.decodedIRData.decodedRawData) {
      case buttonLeft:
        lcd.setCursor(0, 0);
        lcd.print("Schranke");
        lcd.setCursor(0, 1);
        lcd.print("schlie\342t sich!");
        barrier.close();
        break;
      case buttonRight:
        lcd.setCursor(0, 0);
        lcd.print("Schranke");
        lcd.setCursor(0, 1);
        lcd.print("\357ffnet sich");
        barrier.open();
        break;
      case buttonA:
        lcd.setCursor(0, 0);
        lcd.print("100 Schritte");
        lcd.setCursor(0, 1);
        lcd.print("nach links");
        barrier.step(-100);
        break;
      case buttonB:
        lcd.setCursor(0, 0);
        lcd.print("100 Schritte");
        lcd.setCursor(0, 1);
        lcd.print("nach rechts");
        barrier.step(100);
        break;
      case buttonDown:
        barrier.closeAndOpen(lcd, trafficLight, piezo, distanceSensor);
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
  uint32_t intervalMillis = interval * 1000;
  uint32_t startTime = millis(); 

  while (millis() - startTime < intervalMillis) {
    uint32_t remainingMillis = startTime + intervalMillis - millis();

    lcd.setCursor(0, 1);
    lcd.print("Noch ");
    lcd.print(remainingMillis / 1000);
    lcd.print(" Sek. ");

    if (automaticModeControlsLocked && reader.isValidRFIDTagPresent()) {
      automaticModeControlsLocked = false;
    } else if (!automaticModeControlsLocked) {
      // Wurden Daten empfangen?
      if (IrReceiver.decode()) {
        DEBUG_PRINTLN(IrReceiver.decodedIRData.decodedRawData, HEX);
        IrReceiver.resume(); // Nächsten Wert einlesen

        if (IrReceiver.decodedIRData.decodedRawData == buttonUp) {
          mode = Mode::SELECT_MODE; // Zurück zum "Modus wählen"-Menü
          trafficLight.off();
          break;
        } else if (IrReceiver.decodedIRData.decodedRawData == buttonX) {
          automaticModeControlsLocked = true;
        }
      }
    }
  }

  if (mode == Mode::AUTOMATIC) {
    barrier.closeAndOpen(lcd, trafficLight, piezo, distanceSensor);
  }
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
    DEBUG_PRINTLN(IrReceiver.decodedIRData.decodedRawData, HEX);
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

void printDigit(uint8_t digit) {
  if (digit < 10) lcd.print("0");
  lcd.print(digit);
}
