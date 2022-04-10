#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <IRremote.hpp>
#include <TimeLib.h>
#include <DS1307RTC.h>

#include "src/libraries/Debug.h"
#include "src/libraries/TrafficLight.h"
#include "src/libraries/Piezo.h"
#include "src/libraries/DistanceSensor.h"
#include "src/libraries/Barrier.h"
#include "src/libraries/RFIDReader.h"
#include "Constants.h"
#include "ControlsLockedMode.h"
#include "SelectMode.h"
#include "ManualMode.h"
#include "AutomaticMode.h"
#include "ChangeIntervalMode.h"

LiquidCrystal_I2C lcd(lcdAddress, lcdNumColumns, lcdNumRows);
TrafficLight trafficLight(redLedPin, yellowLedPin, greenLedPin);
Piezo piezo(piezoPin, piezoLedPin);
DistanceSensor distanceSensor(triggerPin, echoPin, minimumDistance);
Barrier barrier(spu, stepperPins, stepperSpeed, lcd, trafficLight, piezo, distanceSensor);
RFIDReader reader(sdaPin, rstPin, validIDs, sizeof(validIDs) / sizeof(validIDs[0]));

Mode mode = Mode::CONTROLS_LOCKED;
AutomaticModeInterval automaticModeInterval;

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
      mode = controlsLockedMode(lcd, reader);
      break;
    case Mode::SELECT:
      mode = selectMode(lcd, piezo);
      break;
    case Mode::MANUAL:
      mode = manualMode(lcd, trafficLight, barrier);
      break;
    case Mode::AUTOMATIC:
      mode = automaticMode(lcd, trafficLight, barrier, reader, automaticModeInterval);
      break;
    case Mode::CHANGE_AUTOMATIC_MODE_INTERVAL_DAY:
      mode = changeAutomaticModeInterval(lcd, automaticModeInterval, false);
      break;
    case Mode::CHANGE_AUTOMATIC_MODE_INTERVAL_NIGHT:
      mode = changeAutomaticModeInterval(lcd, automaticModeInterval, true);
      break;
  }
}
