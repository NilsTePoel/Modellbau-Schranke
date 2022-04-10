#include <Arduino.h>
#define USE_IRREMOTE_HPP_AS_PLAIN_INCLUDE
#include <IRremote.hpp>
#include <TimeLib.h>

#include "src/libraries/Debug.h"
#include "AutomaticMode.h"

bool automaticModeControlsLocked = false; // Ist die Steuerung im automatischen Modus aktuell gesperrt?

Mode automaticMode(LiquidCrystal_I2C &lcd, const TrafficLight &trafficLight, Barrier &barrier, RFIDReader &reader, AutomaticModeInterval &interval) {
  Mode newMode = Mode::AUTOMATIC;

  trafficLight.green();

  lcd.setCursor(0, 0);
  lcd.print("Automatisch  ");

  uint16_t currentInterval = (hour() >= 18 || hour() <= 6) ? interval.night : interval.day; // Nachts anderes Intervall
  uint32_t currentIntervalMillis = currentInterval * 1000;
  uint32_t startTime = millis(); 

  while (millis() - startTime < currentIntervalMillis) {
    uint32_t remainingMillis = startTime + currentIntervalMillis - millis();

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
          newMode = Mode::SELECT; // Zurück zum "Modus wählen"-Menü
          trafficLight.off();
          break;
        } else if (IrReceiver.decodedIRData.decodedRawData == buttonX) {
          automaticModeControlsLocked = true;
        }
      }
    }
  }

  if (newMode == Mode::AUTOMATIC) {
    barrier.closeAndOpen();
  }

  return newMode;
}