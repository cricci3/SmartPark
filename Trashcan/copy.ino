#include <U8g2lib.h>
#include <Wire.h>
#include <WiFi.h>
#include <ArduinoMqttClient.h>
#include "mbed.h"
using namespace mbed;
using namespace rtos;

// inserisci nel topic
#define FLOOR_NUMBER_0 0
#define FLOOR_NUMBER_1 1

Thread connectionSetupThread;
bool connection_setup_done = false;
int wifi_status = WL_IDLE_STATUS;
int mqtt_status = 0;

// WiFi credentials
char ssid[] = "parkingG";    // your network SSID
char pass[] = "ciaoClaudio"; // your network password

WiFiClient wifiClient;
MqttClient mqttClient(wifiClient);
const char broker[] = "test.mosquitto.org";
int port = 1883;

// Connect to WiFi and MQTT
void setupConnection() {
    Serial.print("Connecting to WiFi...");
    while (wifi_status != WL_CONNECTED) {
        wifi_status = WiFi.begin(ssid, pass);
        ThisThread::sleep_for(1000);
    }
    Serial.println("Connected to WiFi!");

    Serial.print("Connecting to MQTT broker...");
    while (!mqtt_status) {
        mqtt_status = mqttClient.connect(broker, port);
        ThisThread::sleep_for(1000);
    }
    Serial.println("Connected to MQTT broker!");
    connection_setup_done = true;
}

// Display setup
U8G2_SSD1327_EA_W128128_F_4W_SW_SPI display(U8G2_R0, 13, 5, 10, 7, 8);

// Maximum number of sensors to handle
const uint8_t numSensors = 6;

// Array to store parking status for each sensor
bool parkingStatus[numSensors] = {false};

// Variables to store parsed values
int parkingValue = 0;
int disabledValue = 0;
int electricValue = 0;

// Grid configuration
struct GridConfig {
    const uint8_t cellWidth = 31;
    const uint8_t cellHeight = 31;
    const uint8_t columns = 4;
    const uint8_t rows = 3;
    const uint8_t startX = 0;
    const uint8_t startY = 0;
};

GridConfig grid;

// 'parking', 16x16px
const unsigned char epd_bitmap_parking [] PROGMEM = {
    0x80, 0x01, 0x7f, 0xfe, 0x7f, 0xfe, 0x78, 0x1e, 0x7b, 0xee, 0x7a, 0x2e, 0x7a, 0xae, 0x7a, 0x6e, 
    0x7a, 0x1e, 0x7a, 0xfe, 0x7a, 0xfe, 0x7a, 0xfe, 0x78, 0xfe, 0x7f, 0xfe, 0x7f, 0xfe, 0x80, 0x01
};

// Wheelchair icon, 16x16px
const unsigned char epd_bitmap_disabled_sign [] PROGMEM = {
    0xff, 0xff, 0xf9, 0xff, 0xf6, 0xff, 0xf9, 0xff, 0xfb, 0xff, 0xf8, 0x3f, 0xf0, 0x3f, 0xeb, 0xff, 
    0xdc, 0x0f, 0xdf, 0xf7, 0xdf, 0xd7, 0xdf, 0xd9, 0xef, 0xbb, 0xf0, 0x7f, 0xfd, 0xff, 0xff, 0xff
};

// 'power', 16x16px
const unsigned char epd_bitmap_power [] PROGMEM = {
    0xfc, 0x3f, 0xe0, 0x07, 0xef, 0xf7, 0xef, 0xf7, 0xef, 0xf7, 0xef, 0xf7, 0xee, 0xf7, 0xee, 0xf7, 
    0xec, 0x37, 0xef, 0x77, 0xef, 0x77, 0xef, 0xf7, 0xef, 0xf7, 0xef, 0xf7, 0xef, 0xf7, 0xe0, 0x0f
};

// Display settings
const uint8_t DISPLAY_COLOR_WHITE = 255;
const uint8_t DISPLAY_COLOR_GRAY = 150;

// Function to draw a 16x16 icon in a cell
void drawIcon16x16(uint8_t row, uint8_t col, const unsigned char* iconBitmap) {
    int cellX = grid.startX + (col * grid.cellWidth);
    int cellY = grid.startY + (row * grid.cellHeight);
    
    int iconX = cellX + ((grid.cellWidth - 16) / 2);
    int iconY = cellY + ((grid.cellHeight - 16) / 2);
    
    for(int y = 0; y < 16; y++) {
        for(int x = 0; x < 16; x++) {
            int bytePos = y * 2 + (x >= 8 ? 1 : 0);
            int bit = x % 8;
            
            if(iconBitmap[bytePos] & (1 << (7 - bit))) {
                display.drawPixel(iconX + x, iconY + y);
            }
        }
    }
}

// Function to draw centered text
void drawCenteredText(uint8_t row, uint8_t col, const char* text) {
    int cellX = grid.startX + (col * grid.cellWidth);
    int cellY = grid.startY + (row * grid.cellHeight);
    
    int textWidth = display.getStrWidth(text);
    int textHeight = display.getAscent() - display.getDescent();
    
    int x = cellX + ((grid.cellWidth - textWidth) / 2) + 1;
    int y = cellY + ((grid.cellHeight + textHeight) / 2);
    
    display.drawStr(x, y, text);
}

// Function to draw the grid
void drawGrid() {
    display.setDrawColor(DISPLAY_COLOR_GRAY);
    
    // Vertical lines
    for(uint8_t i = 0; i <= grid.columns; i++) {
        int x = grid.startX + (i * grid.cellWidth);
        display.drawVLine(x, grid.startY, grid.cellHeight * grid.rows);
    }
    
    // Horizontal lines
    for(uint8_t i = 0; i <= grid.rows; i++) {
        int y = grid.startY + (i * grid.cellHeight);
        display.drawHLine(grid.startX, y, grid.cellWidth * grid.columns);
    }
}

// Function to parse the received payload
void parse_update(String payload) {
    // Find the positions of the commas
    int firstComma = payload.indexOf(',');
    int secondComma = payload.lastIndexOf(',');
    
    // Extract and convert the values
    parkingValue = payload.substring(0, firstComma).toInt();
    disabledValue = payload.substring(firstComma + 1, secondComma).toInt();
    electricValue = payload.substring(secondComma + 1).toInt();
    
    // Debug print
    Serial.println("Parsed values:");
    Serial.print("Parking: ");
    Serial.println(parkingValue);
    Serial.print("Disabled: ");
    Serial.println(disabledValue);
    Serial.print("Electric: ");
    Serial.println(electricValue);

    updateParkingDisplay(parkingValue,disabledValue,electricValue);
}

// Function to update display with parking status
void updateParkingDisplay(int parkingValue, int disabledValue, int electricValue) {
    display.firstPage();
    do {
        drawGrid();
        
        display.setFont(u8g2_font_logisoso16_tn);
        display.setDrawColor(DISPLAY_COLOR_WHITE);

        // Draw icons
        drawIcon16x16(0, 1, epd_bitmap_parking);
        drawIcon16x16(0, 2, epd_bitmap_disabled_sign);
        drawIcon16x16(0, 3, epd_bitmap_power);

        // Convert integers to strings for display
        char parkingStr[4];
        char disabledStr[4];
        char electricStr[4];
        
        sprintf(parkingStr, "%d", parkingValue);
        sprintf(disabledStr, "%d", disabledValue);
        sprintf(electricStr, "%d", electricValue);

        // Display the status
        drawCenteredText(1, 1, parkingStr);
        drawCenteredText(1, 2, disabledStr);
        drawCenteredText(1, 3, electricStr);

    } while(display.nextPage());
}

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
    // Process MQTT messages
    mqttClient.poll();
    
    
    // Check for incoming messages
    int messageSize = mqttClient.parseMessage();
    if (messageSize) {
        Serial.println("something arrived");
        String topic = mqttClient.messageTopic();
        Serial.print("Received message on topic: ");
        Serial.println(topic);
        
        // Read full payload
        String payload = "";
        while (mqttClient.available()) {
            payload += (char)mqttClient.read();
        }
        payload.trim();
        
        // Parse the payload into individual values
        parse_update(payload);
    }
    
    // Optional: Add a small delay to prevent excessive processing
    delay(100);
}