#include <SPI.h>

#include "RFIDReader.h"
#include "Debug.h"

RFIDReader::RFIDReader(uint8_t sdaPin, uint8_t rstPin, const uint8_t (*validIDs)[4], uint8_t numberOfValidIDs)
  : m_sdaPin(sdaPin), m_rstPin(rstPin), m_validIDs(validIDs), m_numberOfValidIDs(numberOfValidIDs), m_mfrc522(m_sdaPin, m_rstPin) {
}

void RFIDReader::begin() const {
  SPI.begin();
  m_mfrc522.PCD_Init();
}

bool RFIDReader::isValidRFIDTagPresent() const {
  if (m_mfrc522.PICC_IsNewCardPresent() && m_mfrc522.PICC_ReadCardSerial()) {
    #ifdef DEBUG_MODE
      m_mfrc522.PICC_DumpDetailsToSerial(&m_mfrc522.uid);
    #endif

    // Steuerung entsperren, falls die ID des RFID-Tags einer der eingespeicherten IDs entspricht
    if (m_mfrc522.uid.size == 4) {
      for (uint8_t i = 0; i < m_numberOfValidIDs; i++) {
        bool validID = true;
        for (uint8_t j = 0; j < 4; j++) {
          if (m_mfrc522.uid.uidByte[j] != m_validIDs[i][j]) validID = false;
        }

        if (validID) {
          return true;
        }
      }
    }
  }

  return false;
}