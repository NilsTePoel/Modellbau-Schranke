#define USE_IRREMOTE_HPP_AS_PLAIN_INCLUDE
#include <IRremote.hpp>
#include <TimeLib.h>

#include "src/libraries/Debug.h"
#include "SelectMode.h"

bool selectModeShowTime = false; // Soll die aktuelle Uhrzeit im Menü "Modus wählen" angezeigt werden?

Mode selectMode(LiquidCrystal_I2C &lcd, Piezo &piezo) {
  Mode newMode = Mode::SELECT;

  lcd.setCursor(0, 0);
  lcd.print("Modus w\341hlen    ");
  lcd.setCursor(0, 1);

  if (selectModeShowTime) {
    time_t t = now();
    printDigit(lcd, hour(t)); lcd.print(":");
    printDigit(lcd, minute(t)); lcd.print(":");
    printDigit(lcd, second(t)); lcd.print("        ");
  } else {
    lcd.print("             ");
  }

  // Wurden Daten empfangen?
  if (IrReceiver.decode()) {
    DEBUG_PRINTLN(IrReceiver.decodedIRData.decodedRawData, HEX);
    IrReceiver.resume(); // Nächsten Wert einlesen

    switch (IrReceiver.decodedIRData.decodedRawData) {
      case buttonA:
        newMode = Mode::AUTOMATIC;
        break;
      case buttonB:
        newMode = Mode::MANUAL;
        break;
      case buttonX:
        newMode = Mode::CHANGE_AUTOMATIC_MODE_INTERVAL_DAY;
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

  return newMode;
}

void printDigit(LiquidCrystal_I2C &lcd, uint8_t digit) {
  if (digit < 10) lcd.print("0");
  lcd.print(digit);
}