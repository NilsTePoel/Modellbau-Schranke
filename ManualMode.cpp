#define USE_IRREMOTE_HPP_AS_PLAIN_INCLUDE
#include <IRremote.hpp>

#include "src/libraries/Debug.h"
#include "ManualMode.h"

Mode manualMode(LiquidCrystal_I2C &lcd, const TrafficLight &trafficLight, Barrier &barrier) {
  Mode newMode = Mode::MANUAL;

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
        barrier.closeAndOpen();
        break;
      case buttonUp:
        newMode = Mode::SELECT; // Zurück zum "Modus wählen"-Menü
        trafficLight.off();
        break;
    }
  }

  return newMode;
}
