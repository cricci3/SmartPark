#include <ArduinoMqttClient.h>
#include <WiFi.h>
#include <U8g2lib.h>
#include "Adafruit_VL53L0X.h"

// Thread stuff
#include "mbed.h"
using namespace mbed;
using namespace rtos;

Thread displayThread;

bool display_update_needed = true;

// Display setup
U8G2_SSD1327_EA_W128128_F_4W_SW_SPI display(U8G2_R0, 13, 5, 10, 7, 8);
Adafruit_VL53L0X distanceSensor = Adafruit_VL53L0X();

// Display settings
const uint8_t DISPLAY_COLOR_WHITE = 255;
const uint8_t DISPLAY_COLOR_GRAY = 150;

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
char sensorTopic0[50];
char sensorTopic1[50];

// Parking data
typedef struct {
  bool standard;
  bool handicap;
  bool echarge;
} ParkingStalls;

ParkingStalls stalls [] = {
  {false,false,false},  //floor 0
  {false,false,false}   //floor 1
};

// Display functions
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

// Bitmap definitions
const unsigned char epd_bitmap_parking [] PROGMEM = {
    0x80, 0x01, 0x7f, 0xfe, 0x7f, 0xfe, 0x78, 0x1e, 0x7b, 0xee, 0x7a, 0x2e, 0x7a, 0xae, 0x7a, 0x6e, 
    0x7a, 0x1e, 0x7a, 0xfe, 0x7a, 0xfe, 0x7a, 0xfe, 0x78, 0xfe, 0x7f, 0xfe, 0x7f, 0xfe, 0x80, 0x01
};

const unsigned char epd_bitmap_number_1 [] PROGMEM = {
    0xff, 0xff, 0xff, 0xff, 0xff, 0x7f, 0xfe, 0x3f, 0xfc, 0x3f, 0xff, 0x3f, 0xff, 0x3f, 0xff, 0x3f,
    0xff, 0x3f, 0xff, 0x3f, 0xff, 0x3f, 0xff, 0x3f, 0xfc, 0x1f, 0xfc, 0x1f, 0xff, 0xff, 0xff, 0xff
};

const unsigned char epd_bitmap_number_2 [] PROGMEM = {
    0xff, 0xff, 0xff, 0xff, 0xfc, 0x7f, 0xf8, 0x3f, 0xff, 0x3f, 0xff, 0x3f, 0xff, 0x7f, 0xfe, 0xff,
    0xfd, 0xff, 0xfb, 0xff, 0xf7, 0xff, 0xf8, 0x1f, 0xf8, 0x1f, 0xf8, 0x1f, 0xff, 0xff, 0xff, 0xff
};

const unsigned char epd_bitmap_disabled_sign [] PROGMEM = {
    0xff, 0xff, 0xf9, 0xff, 0xf6, 0xff, 0xf9, 0xff, 0xfb, 0xff, 0xf8, 0x3f, 0xf0, 0x3f, 0xeb, 0xff, 
    0xdc, 0x0f, 0xdf, 0xf7, 0xdf, 0xd7, 0xdf, 0xd9, 0xef, 0xbb, 0xf0, 0x7f, 0xfd, 0xff, 0xff, 0xff
};

const unsigned char epd_bitmap_power [] PROGMEM = {
    0xfc, 0x3f, 0xe0, 0x07, 0xef, 0xf7, 0xef, 0xf7, 0xef, 0xf7, 0xef, 0xf7, 0xee, 0xf7, 0xee, 0xf7, 
    0xec, 0x37, 0xef, 0x77, 0xef, 0x77, 0xef, 0xf7, 0xef, 0xf7, 0xef, 0xf7, 0xef, 0xf7, 0xe0, 0x0f
};

// Drawing functions
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

void drawCenteredText(uint8_t row, uint8_t col, const char* text) {
    int cellX = grid.startX + (col * grid.cellWidth);
    int cellY = grid.startY + (row * grid.cellHeight);
    
    int textWidth = display.getStrWidth(text);
    int textHeight = display.getAscent() - display.getDescent();
    
    int x = cellX + ((grid.cellWidth - textWidth) / 2) + 1;
    int y = cellY + ((grid.cellHeight + textHeight) / 2);
    
    display.drawStr(x, y, text);
}

void drawGrid() {
    display.setDrawColor(DISPLAY_COLOR_GRAY);
    
    for(uint8_t i = 0; i <= grid.columns; i++) {
        int x = grid.startX + (i * grid.cellWidth);
        display.drawVLine(x, grid.startY, grid.cellHeight * grid.rows);
    }
    
    for(uint8_t i = 0; i <= grid.rows; i++) {
        int y = grid.startY + (i * grid.cellHeight);
        display.drawHLine(grid.startX, y, grid.cellWidth * grid.columns);
    }
}

// Display thread function
void displayThreadFunction() {
    Serial.println("Display thread started");
    while (true) {
      display.firstPage();

      do {
          drawGrid();
          
          display.setFont(u8g2_font_logisoso16_tn);
          display.setDrawColor(DISPLAY_COLOR_WHITE);

          drawIcon16x16(0, 1, epd_bitmap_parking);
          drawIcon16x16(0, 2, epd_bitmap_disabled_sign);
          drawIcon16x16(0, 3, epd_bitmap_power);
          drawIcon16x16(1, 0, epd_bitmap_number_1);
          drawIcon16x16(2, 0, epd_bitmap_number_2);

      } while(display.nextPage());
         
      ThisThread::sleep_for(50);
    }
}

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

void parse_payload(String payload, String topic) {
    Serial.println("Parsing payload...");
    int firstComma = payload.indexOf(',');
    int secondComma = payload.lastIndexOf(',');
    
    int parkingValue = payload.substring(0, firstComma).toInt();
    int disabledValue = payload.substring(firstComma + 1, secondComma).toInt();
    int electricValue = payload.substring(secondComma + 1).toInt();
    
    Serial.println("Parsed values:");
    Serial.print("Parking: ");
    Serial.println(parkingValue);
    Serial.print("Disabled: ");
    Serial.println(disabledValue);
    Serial.print("Electric: ");
    Serial.println(electricValue);

    if(topic == "parking/floor0")
      stalls[0] = {parkingValue, disabledValue, electricValue};
    else
      stalls[1] = {parkingValue, disabledValue, electricValue};
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

  // Display setup
  display.begin();
  display.setContrast(255);
  Serial.println("Display initialized");

  snprintf(sensorTopic0, sizeof(sensorTopic0), "parking/floor0");
  mqttClient.subscribe(sensorTopic0);
  Serial.print("Subscribed to topic: ");
  Serial.println(sensorTopic0);


  Serial.println("All topics subscribed!");

  displayThread.start(callback(displayThreadFunction));
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
    String message = "";
    while (mqttClient.available()) {
      // Serial.print((char)mqttClient.read());
      message += (char)mqttClient.read();
    }
    Serial.println(message);
    parse_payload(message, mqttClient.messageTopic());
  }
}