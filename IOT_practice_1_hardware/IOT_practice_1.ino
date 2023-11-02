#include <Arduino.h>
#include "HPMA115_Standard.h"
#include <HTU21D.h>
#include <MKRWAN.h>
#include <time.h>

// OTAA Application Key
#define APP_KEY "f5c73365411457838e56a6abfa0a2131"

// Define a structure data type to save the measures
typedef struct{
  float temperature;
  float humidity;
  uint16_t pm10;
  uint16_t pm25;
} measures_t;

// Global variables initialization
HPMA115_Standard hpm = HPMA115_Standard();
HTU21D sensor;
measures_t myMeasures;
LoRaModem modem;

// Helper function to print current measurement results.
void printResults();
void sendLoraPacket ();

// Timestamp for last time we checked the sensor reading.
// We will use this to avoid requesting data faster than the sensor can
// provide it, which is one reading per second.
unsigned long lastCheck = 0;

void setup() {

  Serial.begin(9600);
  while(!Serial);
  Serial.println("-----------------------------");
  Serial.println("[INFO] Initializing device...");

  // Temperature and humidity initializing
  sensor.begin();
  
  // Particles sensoror initializing
  Serial1.begin(HPMA115_BAUD);
  while(!Serial);
  hpm.begin(&Serial1);
  hpm.stopAutoSend(); // Disabling auto-send. We will check results manually.
  Serial.println("[INFO] Particles sensor started");

  // Initializing LoRa module and connection
  Serial.println("[INFO] Initializing LoRa module..."); 
  if (!modem.begin(EU868)) {
    Serial.println("[CRITICAL] LoRa module did not initialize");
  }
  Serial.print("[INFO] LoRa module activated with EUI: ");
  Serial.println(modem.deviceEUI());
  
  Serial.println("[INFO] Joining network server...");
  int connected = modem.joinOTAA(modem.deviceEUI(), APP_KEY);
      if (!connected) {
        Serial.println("[CRITICAL] Gateway could not be reached.");
        while (!modem.joinOTAA(modem.deviceEUI(), APP_KEY)) {
          modem.joinOTAA(modem.deviceEUI(), APP_KEY);
          delay(5000);
        }
      }
  srand(time(NULL));
  Serial.println("[INFO] Network server reached successfully");

  Serial.println( "[INFO] Setup finished\n" );
}

void loop() {

  // Only ask for a new reading if a second has elapsed.
  if (millis() - lastCheck > 1000) {
    lastCheck = millis();

    if ( hpm.readParticleMeasurementResults_standard() ) {
      myMeasures.pm25 = hpm.getPM25();
      myMeasures.pm10 = hpm.getPM10();
    }

    if( sensor.measure() ){
      myMeasures.temperature = sensor.getTemperature();
      myMeasures.humidity = sensor.getHumidity(); 
    }
  }

  sendLoraPacket();

  printResults();
  // The sensor only sends readings at intervals of one second.
  // There is no point in hammering it with data requests.
  delay(5000);
}

// ------ Functions declaration
void printResults() {
  Serial.print("Temperature: ");
  Serial.print( myMeasures.temperature, 0); 
  Serial.print("°C   Humidity: ");
  Serial.print( myMeasures.humidity, 0); 
  Serial.print("%RH   PM2.5: " );
  Serial.print( myMeasures.pm25, DEC);
  Serial.print("ug/m3   PM10: " );
  Serial.print(myMeasures.pm10, DEC );
  Serial.println( "ug/m3" );
}

void sendLoraPacket() {

  byte data[sizeof(measures_t)];
  memcpy(data,&myMeasures,sizeof(measures_t));

  modem.beginPacket();
  modem.write(data,sizeof(data));
  modem.endPacket();
}

