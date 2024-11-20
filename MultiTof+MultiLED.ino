#include <Wire.h>
#include "Adafruit_VL53L0X.h"

#define TCA9548A_ADDR 0x70

//PIN Custom for led
#define R1 8
#define G1 7
#define B1 6
#define R2 5
#define G2 4
#define B2 3
#define R3 2
#define G3 1
#define B3 0


// to avoid 10000 lines of code we decide to create a structure
struct RGBLed {
    uint8_t R;
    uint8_t G;
    uint8_t B;
};

const RGBLed rgbLeds[] = {
    {R1, G1, B1},
    {R2, G2, B2},
    {R3, G3, B3}
};

// LED 0 - Sensor 0 - Bus 0 : green
// LED 1 - Sensor 1 - Bus 2 : yellow
// LED 2 - Sensor 2 - Bus 6 : purple


const uint8_t sensorBuses[] = {0, 2, 6};
const uint8_t numSensors = sizeof(sensorBuses) / sizeof(sensorBuses[0]);

Adafruit_VL53L0X sensors[numSensors];

// select the bus
void TCA9548A_Select(uint8_t bus) {
    Wire.beginTransmission(TCA9548A_ADDR);
    Wire.write(1 << bus);
    Wire.endTransmission();
}

// general function to set the led 
void setLEDColor(int ledIndex, int color) {
    switch(ledIndex) {
        case 0:  // LED 1
            switch(color) {
                case 0:  // Verde 
                    digitalWrite(R1, LOW);
                    digitalWrite(G1, HIGH);
                    digitalWrite(B1, LOW);
                    break;
                case 1:  // Rosso 
                    digitalWrite(R1, HIGH);
                    digitalWrite(G1, LOW);
                    digitalWrite(B1, LOW);
                    break;
            }
            break;
            
        case 1:  // LED 2 : fundamental to use analogwrite
            switch(color) {
                case 0:  // giallo
                    analogWrite(R2, 255);
                    analogWrite(G2, 165);
                    analogWrite(B2, 0);
                    break;
                case 1:  // rosso
                    analogWrite(R2, 255);
                    analogWrite(G2, 0);
                    analogWrite(B2, 0);
                    break;
            }
            break;
            
        case 2:  // LED 3
            switch(color) {
                case 0:  // blu 
                    analogWrite(R3, 149);
                    analogWrite(G3, 0);
                    analogWrite(B3, 179);
                    break;
                case 1:  // rosso
                    analogWrite(R3, 255);
                    analogWrite(G3, 0);
                    analogWrite(B3, 0);
                    break;
            }
            break;
    }
}

void setup() {
    Serial.begin(115200);
    while (!Serial) delay(1);
    Wire.begin();
    
    // Inizializzazione pin LED
    for (uint8_t i = 0; i < numSensors; i++) {
        pinMode(rgbLeds[i].R, OUTPUT);
        pinMode(rgbLeds[i].G, OUTPUT);
        pinMode(rgbLeds[i].B, OUTPUT);
        setLEDColor(i, 0);  // Imposta il colore default per ogni LED
    }
    
    // Inizializzazione sensori
    for (uint8_t i = 0; i < numSensors; i++) {
        TCA9548A_Select(sensorBuses[i]);
        Serial.print("Inizializzazione sensore ");
        Serial.print(i);
        Serial.print(" su bus ");
        Serial.println(sensorBuses[i]);
        
        if (!sensors[i].begin()) {
            Serial.print("ERRORE: Sensore ");
            Serial.print(i);
            Serial.println(" non inizializzato!");
            while (1);
        } else {
            Serial.print("Sensore ");
            Serial.print(i);
            Serial.println(" OK");
        }
    }
}

void loop() {
    for (uint8_t i = 0; i < numSensors; i++) {
        TCA9548A_Select(sensorBuses[i]);
        VL53L0X_RangingMeasurementData_t measure;
        sensors[i].rangingTest(&measure, false);
        
        Serial.print("Sensore ");
        Serial.print(i);
        Serial.print(" (Bus ");
        Serial.print(sensorBuses[i]);
        Serial.print("): ");
        
        if (measure.RangeStatus != 4) {
            int distance = measure.RangeMilliMeter;
            Serial.print("Distanza: ");
            Serial.print(distance);
            Serial.println(" mm");
            
            if (distance > 0 && distance <= 100) {
                setLEDColor(i, 1);  // Rosso quando oggetto vicino
            } else {
                setLEDColor(i, 0);  // Colore default quando oggetto lontano
            }
        } else {
            Serial.println("Fuori portata");
            setLEDColor(i, 0);  // Colore default quando fuori portata
        }
        
        Serial.println("-------------------");
    }
    delay(100);
}
