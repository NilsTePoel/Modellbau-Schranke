#include <Arduino.h>
#define USE_IRREMOTE_HPP_AS_PLAIN_INCLUDE
#include <IRremote.hpp>

#include "src/libraries/Debug.h"
#include "ChangeIntervalMode.h"

Mode changeAutomaticModeInterval(LiquidCrystal_I2C &lcd, AutomaticModeInterval &interval, bool isNight) {
  Mode newMode = isNight ? Mode::CHANGE_AUTOMATIC_MODE_INTERVAL_NIGHT : Mode::CHANGE_AUTOMATIC_MODE_INTERVAL_DAY;

  uint16_t currentInterval = isNight ? interval.night : interval.day;

  lcd.setCursor(0, 0);
  lcd.print(isNight ? "Interv. (Nacht):" : "Interv. (Tag):  ");
  lcd.setCursor(0, 1);
  lcd.print("< ");
  lcd.print(currentInterval);
  lcd.print(" >  ");

  // Wurden Daten empfangen?
  if (IrReceiver.decode()) {
    DEBUG_PRINTLN(IrReceiver.decodedIRData.decodedRawData, HEX);
    IrReceiver.resume(); // Nächsten Wert einlesen

    switch (IrReceiver.decodedIRData.decodedRawData) {
      case buttonLeft:
        currentInterval = constrain(currentInterval - 1, 1, 999);
        break;
      case buttonRight:
        currentInterval = constrain(currentInterval + 1, 1, 999);
        break;
      case buttonA:
        currentInterval = constrain(currentInterval - 10, 1, 999);
        break;
      case buttonB:
        currentInterval = constrain(currentInterval + 10, 1, 999);
        break;
      case buttonX:
        newMode = isNight ? Mode::SELECT : Mode::CHANGE_AUTOMATIC_MODE_INTERVAL_NIGHT; // Weiter zum nächsten Menü
        break;
      case buttonUp:
        newMode = isNight ? Mode::CHANGE_AUTOMATIC_MODE_INTERVAL_DAY : Mode::SELECT; // Zurück zum letzten Menü
        break;
    }
  }

  if (isNight) {
    interval.night = currentInterval;
  } else { 
    interval.day = currentInterval;
  }

  return newMode;
}