#include <Wire.h>
#include "Adafruit_VL53L0X.h"
#include <WiFi.h>
#include <ArduinoMqttClient.h>

// Thread stuff
#include "mbed.h"
using namespace mbed;
using namespace rtos;

Thread connectionSetupThread;
Thread mqttUpdateThread;

bool connection_setup_done = false;

#define TCA9548A_ADDR 0x70

// WiFi credentials
const char ssid[] = "parkingG";    
const char pass[] = "ciaoClaudio"; 

// WiFi stuff
int status = WL_IDLE_STATUS;

// MQTT broker
const char broker[] = "test.mosquitto.org";
int        port     = 1883;
const char topic[]  = "parking/status";

// PIN Custom for LED
#define R1 8
#define G1 7
#define B1 6
#define R2 5
#define G2 4
#define B2 3
#define R3 2
#define G3 1
#define B3 0

// Structure for RGB LEDs
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

// WiFi and MQTT client
WiFiClient wifiClient;
MqttClient mqttClient(wifiClient);

// Select the bus
void TCA9548A_Select(uint8_t bus) {
    Wire.beginTransmission(TCA9548A_ADDR);
    Wire.write(1 << bus);
    Wire.endTransmission();
}

// General function to set the LED 
void setLEDColor(int ledIndex, int color) {
    switch (ledIndex) {
        case 0:  // LED 1
            switch (color) {
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
            switch (color) {
                case 0:  // Giallo
                    analogWrite(R2, 255);
                    analogWrite(G2, 165);
                    analogWrite(B2, 0);
                    break;
                case 1:  // Rosso
                    analogWrite(R2, 255);
                    analogWrite(G2, 0);
                    analogWrite(B2, 0);
                    break;
            }
            break;
            
        case 2:  // LED 3
            switch (color) {
                case 0:  // Blu 
                    analogWrite(R3, 149);
                    analogWrite(G3, 0);
                    analogWrite(B3, 179);
                    break;
                case 1:  // Rosso
                    analogWrite(R3, 255);
                    analogWrite(G3, 0);
                    analogWrite(B3, 0);
                    break;
            }
            break;
    }
}

// Connect to WiFi and MQTT
void setupConnection() {
    Serial.print("Connecting to WiFi...");
    while (status != WL_CONNECTED) {
        Serial.print("Wifi attempt");
        status = WiFi.begin(ssid, pass);
        Serial.print(".");
        ThisThread::sleep_for(1000);
    }
    Serial.println("Connected to WiFi!");

    Serial.print("Connecting to MQTT broker...");
    while (!mqttClient.connect(broker, port)) {
        Serial.print(".");
        ThisThread::sleep_for(1000);
    }
    Serial.println("Connected to MQTT broker!");

    connection_setup_done = true;
}

void updateMQTT() {
  Serial.print("MQTT thread started, waiting...");
  while (true) {
    ThisThread::sleep_for(10000);

    for (uint8_t i = 0; i < numSensors; i++) {
      // Prepare the MQTT topic
      char sensorTopic[50];
      snprintf(sensorTopic, sizeof(sensorTopic), "parking/sensor%d/status", i + 1);

      if (measure.RangeStatus != 4) {
          int distance = measure.RangeMilliMeter;
          uint8_t currentState = (distance > 10 && distance <= 70) ? 1 : 0;

          // Prepare the MQTT message based on the new state
          char mqttMessage[50];
          if (currentState == 1) {
              snprintf(mqttMessage, sizeof(mqttMessage), "Occupied");
          } else {
              snprintf(mqttMessage, sizeof(mqttMessage), "Free");
          }
          // Send the MQTT message
          mqttClient.beginMessage(sensorTopic);
          mqttClient.print(mqttMessage);
          mqttClient.endMessage();

          Serial.print("Message sent to topic: ");
          Serial.println(sensorTopic);
          Serial.println(mqttMessage);
      }
    }
  }
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

    connectionSetupThread.start(callback(setupConnection));

    // Initialize LED pins
    for (uint8_t i = 0; i < numSensors; i++) {
        pinMode(rgbLeds[i].R, OUTPUT);
        pinMode(rgbLeds[i].G, OUTPUT);
        pinMode(rgbLeds[i].B, OUTPUT);
        setLEDColor(i, 0);  // Set default color for each LED
    }

    // Initialize sensors
    for (uint8_t i = 0; i < numSensors; i++) {
        TCA9548A_Select(sensorBuses[i]);
        Serial.print("Initializing sensor ");
        Serial.print(i);
        Serial.print(" on bus ");
        Serial.println(sensorBuses[i]);

        if (!sensors[i].begin()) {
            Serial.print("ERROR: Sensor ");
            Serial.print(i);
            Serial.println(" not initialized!");
            while (1);
        } else {
            Serial.print("Sensor ");
            Serial.print(i);
            Serial.println(" OK");
        }
    }

    while (connection_setup_done != true) {
        Serial.println("Waiting for connection...");
        ThisThread::sleep_for(1000);
    }
    mqttUpdateThread.start(callback(updateMQTT));
}

void loop() {
    mqttClient.poll();

    // Static array to track the previous state of each parking spot (0: free, 1: occupied)
    static uint8_t previousState[numSensors] = {0};  // Initialize all spots as free

    for (uint8_t i = 0; i < numSensors; i++) {
        TCA9548A_Select(sensorBuses[i]);
        VL53L0X_RangingMeasurementData_t measure;
        sensors[i].rangingTest(&measure, false);

        // Serial.print("Sensor ");
        // Serial.print(i);
        // Serial.print(" (Bus ");
        // Serial.print(sensorBuses[i]);
        // Serial.print("): ");

        if (measure.RangeStatus != 4) {
            int distance = measure.RangeMilliMeter;
            // Serial.print("Distance: ");
            // Serial.print(distance);
            // Serial.println(" mm");

            // Determine the current state: 1 if occupied (distance <= 70), 0 if free
            uint8_t currentState = (distance > 10 && distance <= 70) ? 1 : 0;

            if (currentState != previousState[i]) {  // Check if the state has changed
                Serial.print("Distance: ");
                Serial.print(distance);
                Serial.println(" mm");
                Serial.println("-------------------");
                // Update the previous state
                previousState[i] = currentState;
                
                if (currentState == 1) {
                    setLEDColor(i, 1);  // Red when object is close (occupied)
                } else {
                    setLEDColor(i, 0);  // Default color when object is far (free)
                }
            }
        } else {
            setLEDColor(i, 0);  // Default color when out of range
        }

        // Serial.println("-------------------");
    }

    // Add a short delay to avoid overwhelming the system
    delay(200);
}



