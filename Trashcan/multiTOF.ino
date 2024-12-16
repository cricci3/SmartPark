#include <Wire.h>
#include "Adafruit_VL53L0X.h"

// Indirizzo I2C del multiplexer TCA9548A
#define TCA9548A_ADDR 0x70

// Array per i bus dei sensori
const uint8_t sensorBuses[] = {0, 2, 4}; // Bus con i sensori
const uint8_t numSensors = sizeof(sensorBuses) / sizeof(sensorBuses[0]);

// Array per gli oggetti dei sensori
Adafruit_VL53L0X sensors[numSensors];

// Funzione per selezionare il bus sul TCA9548A
void TCA9548A_Select(uint8_t bus) {
  Wire.beginTransmission(TCA9548A_ADDR);
  Wire.write(1 << bus); // Seleziona il bus (0-7)
  Wire.endTransmission();
}

void setup() {
  Serial.begin(115200);
  while (!Serial) delay(1); // Aspetta l'apertura della porta seriale
  
  Serial.println("Adafruit VL53L0X test con TCA9548A");

  Wire.begin(); // Inizializza il bus I2C
  
  // Inizializzazione di tutti i sensori
  for (uint8_t i = 0; i < numSensors; i++) {
    TCA9548A_Select(sensorBuses[i]); // Seleziona il bus corrente
    if (!sensors[i].begin()) {
      Serial.print("Errore: Sensore su bus ");
      Serial.print(sensorBuses[i]);
      Serial.println(" non inizializzato.");
      while (1); // Blocco in caso di errore
    } else {
      Serial.print("Sensore su bus ");
      Serial.print(sensorBuses[i]);
      Serial.println(" inizializzato correttamente.");
    }
  }
}

void loop() {
  // Lettura di ciascun sensore
  for (uint8_t i = 0; i < numSensors; i++) {
    TCA9548A_Select(sensorBuses[i]); // Seleziona il bus corrente
    
    VL53L0X_RangingMeasurementData_t measure;
    sensors[i].rangingTest(&measure, false); // Misura la distanza

    Serial.print("Bus ");
    Serial.print(sensorBuses[i]);
    Serial.print(": ");

    if (measure.RangeStatus != 4) { // Controlla lo stato della misura
      Serial.print("Distanza (mm): ");
      Serial.println(measure.RangeMilliMeter);
    } else {
      Serial.println("Fuori portata");
    }
  }
  
  delay(500); // Pausa tra le letture
}
