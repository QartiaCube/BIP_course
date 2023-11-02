//#include <Arduino.h>
//#include <stdio.h>
#include "HPMA115_Standard.h"

bool HPMA115_Standard::begin(Stream *stream) {
  hpma = stream;
  return true;
}

bool HPMA115_Standard::stopAutoSend() {
  // Data sheet, Table 6.
  return writeSimpleCommand(0x20);
}

bool HPMA115_Standard::writeSimpleCommand(uint8_t cmdByte) {
  uint8_t cmd[4];

  // Data sheet, Table 6.
  cmd[0] = 0x68;
  cmd[1] = 0x01;  // Length is guaranteed to be one.
  cmd[2] = cmdByte;
  cmd[3] = (0x10000 - (cmd[0] + cmd[1] + cmd[2]) % 256);

  hpma->write(cmd, 4);

  // Block until we get a response.
  while (hpma->available() < 2) {}
  uint8_t resp[2];
  hpma->readBytes(resp, sizeof(resp));

  if (resp[0] == 0xA5 && resp[1] == 0xA5) {
    return true;
  }

  return false;
}
  
bool HPMA115_Standard::readParticleMeasurementResults_standard( ) {
  // Data sheet, Table 6.
  uint8_t cmd[] = { 0x68, 0x01, 0x04, 0x93 };
  hpma->write(cmd, 4);

  // Block until we get a response.
  // A successful response will contain the readings.
  while (hpma->available() < 1) {}
  uint8_t resp_head = hpma->read();
  if (resp_head != 0x40) {
    // Failure. Next byte should be 0x96. Swallow it and die.
    hpma->read();
    return false;
  }

  while (hpma->available() < 1) {}
  if (hpma->read() != 0x05) {
    // Unexpected length.
    return false;
  }

  while (hpma->available() < 1) {}
  if (hpma->read() != 0x04) {
    // Unexpected command
    return false;
  }

  // Read the 4 data bytes. 
  while (hpma->available() < 4) {}
  uint8_t buf[4] = { 0 };
  hpma->readBytes(buf, sizeof(buf));

  while (hpma->available() < 1) {}
  uint32_t expected_sum = hpma->read();

  // Calculate the observed checksum.
  // First subtract the headers ...
  uint32_t actual_sum = 0x10000 - 0x40 - 0x05 - 0x04;
  // ... then the rest of the bytes.
  for (uint8_t i = 0; i < 4; ++i) {
    actual_sum -= buf[i];
  }
  actual_sum %= 256;

  if (actual_sum != expected_sum) {
    return false;
  }

//  == PRINT THE READING  
//   char measure[11];
//   Serial.print("Lectura: ");
//   sprintf(measure, "%02X %02X %02X %02X", buf[0], buf[1], buf[2], buf[3]);
//   Serial.println( measure );
  

  // Everything's great. We can report the readings.
  resultado.pm25 = (buf[0] << 8) + buf[1];
  resultado.pm10 = (buf[2] << 8) + buf[3];

//   Serial.print("PM2.5: " );
//   Serial.print( (buf[0] << 8) + buf[1]), DEC;
//   Serial.print("  PM10: " );
//   Serial.println((buf[2] << 8) + buf[3] , DEC );

  return true;
}

uint16_t HPMA115_Standard::getPM10(){
  return resultado.pm10;
};

uint16_t HPMA115_Standard:: getPM25(){
  return resultado.pm25;
};