#include "ControlsLockedMode.h"

Mode controlsLockedMode(LiquidCrystal_I2C &lcd, RFIDReader &reader) {
  Mode newMode = Mode::CONTROLS_LOCKED;

  lcd.setCursor(0, 0);
  lcd.print("Steuerung");
  lcd.setCursor(0, 1);
  lcd.print("gesperrt");

  // Warten, bis die Steuerung per RFID-Chip entsperrt wurde
  if (reader.isValidRFIDTagPresent()) {
    newMode = Mode::SELECT;
  }

  return newMode;
}