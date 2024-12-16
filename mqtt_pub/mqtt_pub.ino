
#include <ArduinoMqttClient.h>
#if defined(ARDUINO_SAMD_MKRWIFI1010) || defined(ARDUINO_SAMD_NANO_33_IOT) || defined(ARDUINO_AVR_UNO_WIFI_REV2)
  #include <WiFiNINA.h>
#elif defined(ARDUINO_SAMD_MKR1000)
  #include <WiFi101.h>
#elif defined(ARDUINO_ARCH_ESP8266)
  #include <ESP8266WiFi.h>
#elif defined(ARDUINO_PORTENTA_H7_M7) || defined(ARDUINO_NICLA_VISION) || defined(ARDUINO_ARCH_ESP32) || defined(ARDUINO_GIGA) || defined(ARDUINO_OPTA)
  #include <WiFi.h>
#elif defined(ARDUINO_PORTENTA_C33)
  #include <WiFiC3.h>
#elif defined(ARDUINO_UNOR4_WIFI)
  #include <WiFiS3.h>
#endif

///////please enter your sensitive data in the Secret tab/arduino_secrets.h
char ssid[] = "parkingG";    // your network SSID (name)
char pass[] = "ciaoClaudio";    // your network password (use for WPA, or use as key for WEP)


// To connect with SSL/TLS:
// 1) Change WiFiClient to WiFiSSLClient.
// 2) Change port value from 1883 to 8883.
// 3) Change broker value to a server with a known SSL/TLS root certificate 
//    flashed in the WiFi module.

WiFiClient wifiClient;
MqttClient mqttClient(wifiClient);

const char broker[] = "test.mosquitto.org";
int        port     = 1883;
const char topic[]  = "arduino/simple";

const long interval = 1000;
unsigned long previousMillis = 0;

int count = 0;

void setup() {
  //Initialize serial and wait for port to open:
  Serial.begin(9600);
  while (!Serial) {
    ; // wait for serial port to connect. Needed for native USB port only
  }

  // attempt to connect to WiFi network:
  Serial.print("Attempting to connect to WPA SSID: ");
  Serial.println(ssid);
  while (WiFi.begin(ssid, pass) != WL_CONNECTED) {
    // failed, retry
    Serial.print(".");
    delay(5000);
  }

  Serial.println("You're connected to the network");
  Serial.println();

  // You can provide a unique client ID, if not set the library uses Arduino-millis()
  // Each client must have a unique client ID
  // mqttClient.setId("clientId");

  // You can provide a username and password for authentication
  // mqttClient.setUsernamePassword("username", "password");

  Serial.print("Attempting to connect to the MQTT broker: ");
  Serial.println(broker);

  if (!mqttClient.connect(broker, port)) {
    Serial.print("MQTT connection failed! Error code = ");
    Serial.println(mqttClient.connectError());

    while (1);
  }

  Serial.println("You're connected to the MQTT broker!");
  Serial.println();
}

void loop() {
    mqttClient.poll();

    // Array to keep track of the previous state of each parking spot (0: free, 1: occupied)
    static uint8_t previousState[numSensors] = {0};  // Initialize all spots as free

    for (uint8_t i = 0; i < numSensors; i++) {
        TCA9548A_Select(sensorBuses[i]);
        VL53L0X_RangingMeasurementData_t measure;
        sensors[i].rangingTest(&measure, false);

        Serial.print("Sensor ");
        Serial.print(i);
        Serial.print(" (Bus ");
        Serial.print(sensorBuses[i]);
        Serial.print("): ");

        if (measure.RangeStatus != 4) {
            int distance = measure.RangeMilliMeter;
            Serial.print("Distance: ");
            Serial.print(distance);
            Serial.println(" mm");

            // Determine the current state: 1 if occupied (distance <= 70), 0 if free
            uint8_t currentState = (distance > 0 && distance <= 70) ? 1 : 0;

            if (currentState != previousState[i]) {  // Check if the state has changed
                // Update the previous state
                previousState[i] = currentState;

                // Prepare the MQTT topic
                char sensorTopic[50];
                snprintf(sensorTopic, sizeof(sensorTopic), "parking/sensor%d/status", i + 1);

                // Prepare the MQTT message based on the new state
                char mqttMessage[50];
                if (currentState == 1) {
                    snprintf(mqttMessage, sizeof(mqttMessage), "Occupied");
                    setLEDColor(i, 1);  // Red when object is close (occupied)
                } else {
                    snprintf(mqttMessage, sizeof(mqttMessage), "Free");
                    setLEDColor(i, 0);  // Default color when object is far (free)
                }

                // Send the MQTT message if the state has changed
                mqttClient.beginMessage(sensorTopic);
                mqttClient.print(mqttMessage);
                mqttClient.endMessage();

                Serial.print("Message sent to topic: ");
                Serial.println(sensorTopic);
                Serial.println(mqttMessage);
            }
        } else {
            Serial.println("Out of range");
            setLEDColor(i, 0);  // Default color when out of range
        }

        Serial.println("-------------------");
    }
    delay(100);
}
