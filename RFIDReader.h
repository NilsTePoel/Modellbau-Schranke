#pragma once

#include <Arduino.h>
#include <MFRC522.h>

class RFIDReader {
public:
  RFIDReader(uint8_t sdaPin, uint8_t rstPin, const uint8_t (*validIDs)[4], uint8_t numberOfValidIDs);

  void begin() const;

  bool isValidRFIDTagPresent() const;

private:
  const uint8_t m_sdaPin;
  const uint8_t m_rstPin;
  const uint8_t (*m_validIDs)[4];
  const uint8_t m_numberOfValidIDs;
  MFRC522 m_mfrc522;
};