#include <Wire.h>
#include "Adafruit_VL53L0X.h"
#include <WiFi.h>

#include "arduino_secrets.h"

#include "mbed.h"
using namespace mbed;
using namespace rtos;

Thread thread;

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

WiFiClient client;

// Parking status
typedef struct {
    int standard;
    int handicap;
    int echarge;
} ParkingStalls;

ParkingStalls stalls;

///////please enter your sensitive data in the Secret tab/arduino_secrets.h
char ssid[] = SECRET_SSID;        // your network SSID (name)
char pass[] = SECRET_PASS;        // your network password (use for WPA, or use as key for WEP)
// int keyIndex = 0;                 // your network key Index number (needed only for WEP)

int status = WL_IDLE_STATUS;
// if you don't want to use DNS (and reduce your sketch size)
// use the numeric IP instead of the name for the server:
IPAddress server(10, 0, 0, 1);  // IP address for example.com (no DNS)


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
                    analogWrite(R2, 204);
                    analogWrite(G2, 136);
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

void sendPostRequest(int floorID, int stallType, int counter, IPAddress server) {
    String jsonPayload = "{\"floor_id\": "+String(floorID) +
    ", \"stall_type\": " + String(stallType) +
    ", \"counter\": " + String(counter) + "}";

    Serial.println("Sent JSON payload:");
    Serial.println(jsonPayload);

    // Make a HTTP request:
    client.println("POST /index.html HTTP/1.1");
    client.print("Host: ");
    client.println(server);
    client.println("Content-Type: application/json");
    client.print("Content-Length: ");
    client.println(jsonPayload.length());
    client.println();
    client.println(jsonPayload);
    client.println();
    client.println();
}

void printWifiStatus() {
  // print the SSID of the network you're attached to:
  Serial.print("SSID: ");
  Serial.println(WiFi.SSID());

  // print your board's IP address:
  IPAddress ip = WiFi.localIP();
  Serial.print("IP Address: ");
  Serial.println(ip);

  // print the received signal strength:
  long rssi = WiFi.RSSI();
  Serial.print("signal strength (RSSI):");
  Serial.print(rssi);
  Serial.println(" dBm");
}

void setup() {
    Serial.begin(115200);
    while (!Serial) delay(1);
    Wire.begin();

    // check for the WiFi module:
    if (WiFi.status() == WL_NO_SHIELD) {
      Serial.println("Communication with WiFi module failed!");
      // don't continue
      while (true);
    }

    // attempt to connect to Wifi network:
    while (status != WL_CONNECTED) {
      Serial.print("Attempting to connect to SSID: ");
      Serial.println(ssid);
      // Connect to WPA/WPA2 network. Change this line if using open or WEP network:
      status = WiFi.begin(ssid, pass);

      // wait 3 seconds for connection:
      delay(3000);
    }
    Serial.println("Connected to wifi");
    printWifiStatus();

    Serial.println("\nStarting connection to server...");
    // if you get a connection, report back via serial:
    // if (client.connect(server, 80)) {
    //   Serial.println("connected to server");

    //   sendPostRequest(1, 2, 0, server);
    // }

    // Initialize stalls
    stalls.standard = 1;
    stalls.handicap = 1;
    stalls.echarge = 1;
    
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

    thread.start(callback(updateController));
}

void updateController() {
  while (true) {
    Serial.println("Started thread");
    Serial.println("Thread sleeping...");
    ThisThread::sleep_for(20000);
    Serial.println("Wait time elapsed");

    // attempt to connect to Wifi network:
    while (status != WL_CONNECTED) {
      Serial.print("Attempting to connect to SSID: ");
      Serial.println(ssid);
      // Connect to WPA/WPA2 network. Change this line if using open or WEP network:
      status = WiFi.begin(ssid, pass);

      // wait 3 seconds for connection:
      delay(3000);
    }
    Serial.println("Connected to wifi");
    printWifiStatus();

    for (int i = 0; i < 3; i ++) {
      if (client.connect(server, 80)) {
        Serial.println("connected to server");
        switch (i) {
          case 0:
            sendPostRequest(0, i, stalls.standard, server);
            Serial.println("Update sent");
            ThisThread::sleep_for(2000);
            break;
          case 1:
            sendPostRequest(0, i, stalls.handicap, server);
            Serial.println("Update sent");
            ThisThread::sleep_for(2000);
            break;
          case 2:
            sendPostRequest(0, i, stalls.echarge, server);
            Serial.println("Update sent");
            ThisThread::sleep_for(2000);
            break;
        }
    }
    }

    Serial.println("Thread run done");
  }
}

void loop() {
    // if there are incoming bytes available
    // from the server, read them and print them:
    while (client.available()) {
      char c = client.read();
      Serial.write(c);
    }

    // // if the server's disconnected, stop the client:
    // if (!client.connected()) {
    //   Serial.println();
    //   Serial.println("disconnecting from server.");
    //   client.stop();

    //   // do nothing forevermore:
    //   while (true);
    // }

    for (uint8_t i = 0; i < numSensors; i++) {
        TCA9548A_Select(sensorBuses[i]);
        VL53L0X_RangingMeasurementData_t measure;
        sensors[i].rangingTest(&measure, false);
        
        // Serial.print("Sensore ");
        // Serial.print(i);
        // Serial.print(" (Bus ");
        // Serial.print(sensorBuses[i]);
        // Serial.print("): ");
        
        if (measure.RangeStatus != 4) {
            int distance = measure.RangeMilliMeter;
            // Serial.print("Distanza: ");
            // Serial.print(distance);
            // Serial.println(" mm");
            
            if (distance > 0 && distance <= 70) {
                setLEDColor(i, 1);  // Rosso quando oggetto vicino
                switch (i) {
                  case 0:
                    stalls.standard = 0;
                    break;
                  case 1:
                    stalls.handicap = 0;
                    break;
                  case 2:
                    stalls.echarge = 0;
                    break;
                }
            } else {
                setLEDColor(i, 0);  // Colore default quando oggetto lontano
                switch (i) {
                  case 0:
                    stalls.standard = 1;
                    break;
                  case 1:
                    stalls.handicap = 1;
                    break;
                  case 2:
                    stalls.echarge = 1;
                    break;
                }
            }
        } else {
            // Serial.println("Fuori portata");
            setLEDColor(i, 0);  // Colore default quando fuori portata
        }
        
        // Serial.println("-------------------");
    }
    delay(100);
}
