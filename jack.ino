#include <ArduinoMqttClient.h>
#include <WiFi.h>  // Include WiFi library for ESP32

// WiFi credentials
char ssid[] = "parkingG";    // your network SSID
char pass[] = "ciaoClaudio"; // your network password

WiFiClient wifiClient;
MqttClient mqttClient(wifiClient);

const char broker[] = "test.mosquitto.org";
int port = 1883;

// Maximum number of sensors to handle
const uint8_t numSensors = 3;

// Array to store sensor topic names
char sensorTopic[50];

// Function to connect to WiFi
void connectToWiFi() {
  Serial.print("Connecting to WiFi...");
  while (WiFi.begin(ssid, pass) != WL_CONNECTED) {
    Serial.print(".");
    delay(1000);
  }
  Serial.println("Connected to WiFi!");
}

// Function to connect to MQTT broker
void connectToMQTT() {
  Serial.print("Connecting to MQTT broker...");
  while (!mqttClient.connect(broker, port)) {
    Serial.print(".");
    delay(1000);
  }
  Serial.println("Connected to MQTT broker!");
}

void setup() {
  // Initialize serial and wait for the port to open
  Serial.begin(9600);
  while (!Serial) {
    ; // wait for serial port to connect
  }

  // Connect to WiFi and MQTT broker
  connectToWiFi();
  connectToMQTT();

  snprintf(sensorTopic, sizeof(sensorTopic), "parking/floor0");
  mqttClient.subscribe(sensorTopic);
  Serial.print("Subscribed to topic: ");
  Serial.println(sensorTopic);


  Serial.println("All topics subscribed!");
}

void loop() {
  // Regularly call poll() to process incoming messages
  mqttClient.poll();

  // Check for incoming messages on subscribed topics
  int messageSize = mqttClient.parseMessage();
  if (messageSize) {
    Serial.print("Received a message on topic: ");
    Serial.println(mqttClient.messageTopic());

    Serial.print("Message size: ");
    Serial.println(messageSize);

    Serial.print("Message: ");
    while (mqttClient.available()) {
      Serial.print((char)mqttClient.read());
    }
    Serial.println();
  }
}