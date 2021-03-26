#include <IRremote.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <Stepper.h>
#include <SPI.h>
#include <MFRC522.h>
#include <TimeLib.h>
#include <DS1307RTC.h>

// #define DEBUG

LiquidCrystal_I2C lcd(0x27, 16, 2);

// Infrarot-Receiver
const int receiverPin = 11;
const unsigned long buttonA = 3125149440;
const unsigned long buttonB = 3091726080;
const unsigned long buttonX = 3208707840;
const unsigned long buttonLeft = 3141861120;
const unsigned long buttonRight = 3158572800;
const unsigned long buttonDown = 3927310080;
const unsigned long buttonUp = 3108437760;

// Schrittmotor
const int spu = 2048; // Schritte pro Umdrehung
Stepper motor(spu, 3, 5, 4, 6);

// LEDs für die Ampelsteuerung
const int redLedPin = 28;
const int yellowLedPin = 24;
const int greenLedPin = 26;

// Piezo
const int piezoPin = 8;
boolean piezoEnabled = false;

// Ultraschallsensor
const int triggerPin = 10;
const int echoPin = A0;
const int minimumDistance = 5; // Minimale Entfernung zur Schranke in Zentimetern

// RFID Kit
const int sdaPin = 53;
const int rstPin = 2;
const int validIDs[2][4] = {{0x3A, 0x26, 0xC5, 0x5C}, {0x87, 0x5B, 0xCF, 0x93}}; 
MFRC522 mfrc522(sdaPin, rstPin);

int automaticModeInterval = 30; // Anzahl der Sekunden, in der die Schranke geöffnet ist
int automaticModeIntervalNight = 60; // Anzahl der Sekunden, in der die Schranke nachts (zw. 18 und 5 Uhr) geöffnet ist
int mode = 0; // 0 = noch nicht ausgewählt, 1 = manuell, 2 = automatisch, 3 = "automaticModeInterval" anpassen, 4 = "automaticModeIntervalNight" anpassen
boolean controlsLocked = true;
boolean showTime = false;

void setup() {
  #ifdef DEBUG
    Serial.begin(9600);
  #endif

  // LCD einrichten
  lcd.init();
  lcd.backlight();
  
  IrReceiver.begin(receiverPin, DISABLE_LED_FEEDBACK); // IR-Empfänger aktivieren

  motor.setSpeed(5); // Motor-Geschwindigkeit in Umdrehungen pro Minute festlegen

  // LED-Pins
  pinMode(redLedPin, OUTPUT);
  pinMode(yellowLedPin, OUTPUT);
  pinMode(greenLedPin, OUTPUT);

  // Ultraschallsensor-Pins
  pinMode(triggerPin, OUTPUT);
  pinMode(echoPin, INPUT);

  pinMode(piezoPin, OUTPUT); // Piezo-Pin
  pinMode(LED_BUILTIN, OUTPUT); // Eingebaute LED (zeigt Piezo-Status an)

  // RFID-Kit einrichten
  SPI.begin();
  mfrc522.PCD_Init();

  // RTC
  setSyncProvider(RTC.get);
}

void loop() {
  // Warten, bis die Steuerung per RFID-Chip entsperrt wurde
  while (controlsLocked) {
    lcd.setCursor(0, 0);
    lcd.print("Steuerung");
    lcd.setCursor(0, 1);
    lcd.print("gesperrt");
    
    unlockControls();
  }
  
  if (mode == 0) {
    lcd.setCursor(0, 0);
    lcd.print("Modus w\341hlen    ");
    lcd.setCursor(0, 1);
    if (showTime) {
      lcd.print(hour());
      lcd.print(":");
      lcd.print(minute());
      lcd.print(":");
      lcd.print(second());
      lcd.print("        ");
    } else {
      lcd.print("             ");
    }
  
    // Wurden Daten empfangen?
    if (IrReceiver.decode()) {
      #ifdef DEBUG
        Serial.println(IrReceiver.decodedIRData.decodedRawData, DEC);
      #endif
      IrReceiver.resume(); // Nächsten Wert einlesen
  
      switch (IrReceiver.decodedIRData.decodedRawData) {
        case buttonA:
          mode = 1; // Manueller Modus
          break;
        case buttonB:
          mode = 2; // Automatischer Modus
          break;
        case buttonX:
          mode = 3; // Intervall ändern
          break;
        case buttonLeft:
          piezoEnabled = false;
          digitalWrite(LED_BUILTIN, LOW);
          break;
        case buttonRight:
          piezoEnabled = true;
          digitalWrite(LED_BUILTIN, HIGH);
          break;
        case buttonDown:
          showTime = !showTime;
          break;
      }
    }
  } else if (mode == 1) {
    manualMode();
  } else if (mode == 2) {
    automaticMode();
  } else if (mode == 3) {
    changeAutomaticModeInterval();
  } else if (mode == 4) {
    changeAutomaticModeIntervalNight();
  }

  // Uhrzeit ausgeben
  #ifdef DEBUG
    if (millis() % 10 == 0) {
      Serial.print(hour()); Serial.print(":");
      Serial.print(minute()); Serial.print(":");
      Serial.println(second());
    }
  #endif
}

void manualMode() {
  digitalWrite(greenLedPin, HIGH);
  lcd.setCursor(0, 0);
  lcd.print("Manuell         ");
  lcd.setCursor(0, 1);
  lcd.print("                ");
  
  // Wurden Daten empfangen?
  if (IrReceiver.decode()) {
    #ifdef DEBUG
      Serial.println(IrReceiver.decodedIRData.decodedRawData, DEC);
    #endif
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
        closeBarrier();
        break;
      case buttonUp:
        mode = 0; // Zurück zum "Modus wählen"-Menü
        digitalWrite(greenLedPin, LOW);
        break;
    }
  }
}

void automaticMode() {
  digitalWrite(greenLedPin, HIGH);
  lcd.setCursor(0, 0);
  lcd.print("Automatisch  ");

  int interval = (hour() >= 18 || hour() <= 5) ? automaticModeIntervalNight : automaticModeInterval; // Nachts anderes Intervall

  for (int remainingTime = interval; remainingTime >= 0; remainingTime--) {
    lcd.setCursor(0, 1);
    lcd.print("Noch ");
    lcd.print(remainingTime);
    lcd.print(" Sek. ");

    if (controlsLocked) {
      unlockControls();
    } else {
      // Wurden Daten empfangen?
      if (IrReceiver.decode()) {
        #ifdef DEBUG
          Serial.println(IrReceiver.decodedIRData.decodedRawData, DEC);
        #endif
        IrReceiver.resume(); // Nächsten Wert einlesen
    
        if (IrReceiver.decodedIRData.decodedRawData == buttonUp) {
          mode = 0; // Zurück zum "Modus wählen"-Menü
          digitalWrite(greenLedPin, LOW);
          break;
        } else if (IrReceiver.decodedIRData.decodedRawData == buttonX) {
          controlsLocked = true;
        }
      }
    }

    if (remainingTime == 0) closeBarrier();
    delay(1000);
  }
}

void changeAutomaticModeInterval() {
  lcd.setCursor(0, 0);
  lcd.print("Intervall:      ");
  lcd.setCursor(0, 1);
  lcd.print("< ");
  lcd.print(automaticModeInterval);
  lcd.print(" >  ");

  // Wurden Daten empfangen?
  if (IrReceiver.decode()) {
    #ifdef DEBUG
      Serial.println(IrReceiver.decodedIRData.decodedRawData, DEC);
    #endif
    IrReceiver.resume(); // Nächsten Wert einlesen

    switch (IrReceiver.decodedIRData.decodedRawData) {
      case buttonLeft:
        automaticModeInterval = constrain(automaticModeInterval - 1, 1, 999);
        break;
      case buttonRight:
        automaticModeInterval = constrain(automaticModeInterval + 1, 1, 999);
        break;
      case buttonA:
        automaticModeInterval = constrain(automaticModeInterval - 10, 1, 999);
        break;
      case buttonB:
        automaticModeInterval = constrain(automaticModeInterval + 10, 1, 999);
        break;
      case buttonX:
        mode = 4; // Weiter zum "Intervall (nachts) ändern"-Menü
        break;
      case buttonUp:
        mode = 0; // Zurück zum "Modus wählen"-Menü
        break;
    }
  }
}

void changeAutomaticModeIntervalNight() {
  lcd.setCursor(0, 0);
  lcd.print("Interv. (nachts)");
  lcd.setCursor(0, 1);
  lcd.print("< ");
  lcd.print(automaticModeIntervalNight);
  lcd.print(" >  ");

  // Wurden Daten empfangen?
  if (IrReceiver.decode()) {
    #ifdef DEBUG
      Serial.println(IrReceiver.decodedIRData.decodedRawData, DEC);
    #endif
    IrReceiver.resume(); // Nächsten Wert einlesen

    switch (IrReceiver.decodedIRData.decodedRawData) {
      case buttonLeft:
        automaticModeIntervalNight = constrain(automaticModeIntervalNight - 1, 1, 999);
        break;
      case buttonRight:
        automaticModeIntervalNight = constrain(automaticModeIntervalNight + 1, 1, 999);
        break;
      case buttonA:
        automaticModeIntervalNight = constrain(automaticModeIntervalNight - 10, 1, 999);
        break;
      case buttonB:
        automaticModeIntervalNight = constrain(automaticModeIntervalNight + 10, 1, 999);
        break;
      case buttonX:
        mode = 0; // Weiter zum "Modus wählen"-Menü
        break;
      case buttonUp:
        mode = 3; // Zurück zum "Intervall ändern"-Menü
        break;
    }
  }
}

void closeBarrier() {
  lcd.setCursor(0, 0);
  lcd.print("Schranke        ");
  lcd.setCursor(0, 1);
  lcd.print("schlie\342t sich!  ");

  // Ampel schaltet erst auf gelb und dann auf rot um
  digitalWrite(greenLedPin, LOW);
  digitalWrite(yellowLedPin, HIGH);
  if (piezoEnabled) digitalWrite(piezoPin, HIGH); // Piezo einschalten
  delay(500);
  if (piezoEnabled) digitalWrite(piezoPin, LOW); // Piezo ausschalten
  delay(2500);
  digitalWrite(redLedPin, HIGH);
  digitalWrite(yellowLedPin, LOW);
  
  // Die Schranke soll sich nicht schließen, wenn sich ein Objekt unter ihr befindet
  while (isBlockedByObstacle()) {
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
  digitalWrite(yellowLedPin, HIGH);
  delay(3000);
  digitalWrite(greenLedPin, HIGH);
  digitalWrite(redLedPin, LOW);
  digitalWrite(yellowLedPin, LOW);
}

boolean isBlockedByObstacle() {
  digitalWrite(triggerPin, LOW);
  delay(5);
  digitalWrite(triggerPin, HIGH);
  delay(10);
  digitalWrite(triggerPin, LOW);

  int duration = pulseIn(echoPin, HIGH); // Dauer, bis der Schall zum Ultraschallsensor zurückkehrt
  int distance = 0.03432 * (duration / 2); // s = v * t (in cm)

  return distance < minimumDistance;
}

void disableMotor() {
  for (int pin = 3; pin <= 6; pin++) {
    digitalWrite(pin, LOW);
  }
}

void unlockControls() {
  if (mfrc522.PICC_IsNewCardPresent() && mfrc522.PICC_ReadCardSerial()) {
    #ifdef DEBUG
      Serial.print("ID des RFID-Tags: ");
      for (byte i = 0; i < mfrc522.uid.size; i++) {
        Serial.print(mfrc522.uid.uidByte[i], HEX);
        Serial.print(" ");
      }
      Serial.println();
    #endif

    // Steuerung entsperren, falls die ID des RFID-Tags einer der eingespeicherten IDs entspricht
    if (mfrc522.uid.size == 4) {
      for (byte i = 0; i < 2; i++) {
        boolean validID = true;
        for (byte j = 0; j < 4; j++) {
          if (mfrc522.uid.uidByte[j] != validIDs[i][j]) validID = false;
        }

        if (validID) {
          controlsLocked = false;
          break;
        }
      }
    }
  }
}
