#ifndef HPMA115_STANDARD_H_
#define HPMA115_STANDARD_H_

#include <HPMA115_Compact.h>
#include <stddef.h>

typedef struct 
{
  uint16_t pm25;
  uint16_t pm10;
}particles_result_t;


class HPMA115_Standard : HPMA115_Compact {
  public:
    HPMA115_Standard():HPMA115_Compact(){};
    bool begin(Stream *stream);
    bool stopAutoSend();
    bool readParticleMeasurementResults_standard ();
    uint16_t getPM10();
    uint16_t getPM25();

  private:
    Stream *hpma = NULL;
    bool writeSimpleCommand(uint8_t cmdByte);
    particles_result_t resultado;
};

#endif  //HPMA115_STANDARD_H_
