# Modellbau-Schranke

## Menü "Modus wählen"

Die Tastencodes müssen je nach Infrarot-Fernbedienung in der Datei `Constants.h` angepasst werden:

```C++
const uint32_t buttonA = 0xBA45FF00;
const uint32_t buttonB = 0xB847FF00;
const uint32_t buttonX = 0xBF40FF00;
const uint32_t buttonLeft = 0xBB44FF00;
const uint32_t buttonRight = 0xBC43FF00;
const uint32_t buttonDown = 0xEA15FF00;
const uint32_t buttonUp = 0xB946FF00;
```

| Taste  | Funktion                                                         |
| ------ | ---------------------------------------------------------------- |
| A      | [Automatischer Modus](#automatischer-modus)                      |
| B      | [Manueller Modus](#manueller-modus)                              |
| X      | Öffnet das Menü ["Intervall anpassen"](#menü-intervall-anpassen) |
| Links  | Warnsignal (Piezo) beim Schließen der Schranke ausschalten       |
| Rechts | Warnsignal (Piezo) beim Schließen der Schranke einschalten       |
| Unten  | Anzeige der Uhrzeit in diesem Menü ein-/ausschalten              |

## Automatischer Modus

Im automatischen Modus schließt sich die Schranke in bestimmten Zeitabständen, die im Menü ["Intervall anpassen"](#menü-intervall-anpassen) geändert werden können. Die Ampel wird dabei automatisch gesteuert. Die Hinderniserkennung verhindert, dass sich die Schranke schließt, wenn sich ein Objekt (z. B. ein Fahrzeug) unter ihr befindet.

| Taste | Funktion                                             |
| ----- | ---------------------------------------------------- |
| X     | Steuerung sperren                                    |
| Oben  | Zurück zum Menü ["Modus wählen"](#menü-modus-wählen) |

## Manueller Modus

Im manuellen Modus wird die Schranke per Fernbedienung gesteuert.

| Taste  | Funktion                                                   |
| ------ | ---------------------------------------------------------- |
| Links  | Schranke schließen                                         |
| Rechts | Schranke öffnen                                            |
| A      | Schrittmotor um 100 Schritte nach links bewegen            |
| B      | Schrittmotor um 100 Schritte nach rechts bewegen           |
| Unten  | Schranke schließen und öffnen (wie im automatischen Modus) |
| Oben   | Zurück zum Menü "[Modus wählen"](#menü-modus-wählen)       |

## Menü "Intervall anpassen"

Hier kann man die Zeit (in Sekunden) einstellen, in der die Schranke im automatischen Modus geöffnet ist, bevor sie sich wieder schließt. Es werden nacheinander zwei verschiedene Zeiten eingestellt. Die erste wird tagsüber (6 - 18 Uhr) verwendet, die zweite nachts. So kann man z. B. erreichen, dass sich die Schranke nachts seltener öffnet.
Die Eingaben werden jeweils mit "X" bestätigt.

## Zugangssperre

Beim Einschalten ist die Steuerung gesperrt und muss erst mit einem RFID-Tag mit gültiger ID entsperrt werden. Die gültigen IDs können im folgenden Array innerhalb der Datei `Constants.h` festgelegt werden:

```C++
const uint8_t validIDs[][4] = {{0x3A, 0x26, 0xC5, 0x5C}, {0x87, 0x5B, 0xCF, 0x93}};
```

Auch im automatischen Modus hat man die Möglichkeit, die Steuerung zu sperren.

## Aufbau

Die Pin-Belegung kann über die Konstanten in der Datei `Constants.h` geändert werden.

| Bauteil            | Pin-Belegung                                      |
| ------------------ | ------------------------------------------------- |
| Infrarot-Empfänger | 11                                                |
| Schrittmotor       | 3, 4, 5, 6                                        |
| Rote LED           | 28                                                |
| Gelbe LED          | 24                                                |
| Grüne LED          | 26                                                |
| Piezo              | 8                                                 |
| Ultraschallsensor  | 10 (Trigger), A0 (Echo)                           |
| RFID-Empfänger     | 2 (RST), 50 (MISO), 51 (MOSI), 52 (SCK), 53 (SDA) |
| LCD                | SDA, SCL                                          |
| Real Time Clock    | SDA, SCL                                          |

## Verwendete Bibliotheken

* [IRremote 3.6.1](https://github.com/Arduino-IRremote/Arduino-IRremote)
* [LiquidCrystal I2C 1.1.2](https://github.com/johnrickman/LiquidCrystal_I2C)
* [MFRC522 1.4.10](https://github.com/miguelbalboa/rfid)
* [Time Library 1.6.1](https://github.com/PaulStoffregen/Time)/[DS1307RTC 1.4.1](https://github.com/PaulStoffregen/DS1307RTC)
