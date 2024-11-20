#include <U8g2lib.h>
#include <Wire.h>
#include "Adafruit_VL53L0X.h"

#ifdef U8X8_HAVE_HW_SPI
#include <SPI.h>
#endif

#ifdef U8X8_HAVE_HW_I2C
#include <Wire.h>
#endif

// Initialize OLED display
U8G2_SSD1327_EA_W128128_F_4W_SW_SPI u8g2(U8G2_R0, /* clock=*/ 13, /* data=*/ 5, /* cs=*/ 10, /* dc=*/ 7, /* reset=*/ 8);

// Initialize VL53L0X sensor
Adafruit_VL53L0X lox = Adafruit_VL53L0X();

// Display constants
#define WHITE 255
#define YELLOW 200
#define GRAY 150

// Table structure
const int COL_WIDTH = 28;     // Column width
const int ROW_HEIGHT = 28;    // Row height
const int START_X = 0;        // Initial X position
const int START_Y = 0;        // Initial Y position
const int NUM_COLS = 3;       // Number of columns
const int NUM_ROWS = 3;       // Number of rows (3 instead of 4)

void drawTable() {
    // Vertical lines
    for(int i = 0; i <= NUM_COLS; i++) {
        u8g2.drawVLine(START_X + (i * COL_WIDTH), START_Y, ROW_HEIGHT * NUM_ROWS);
    }
    
    // Horizontal lines
    for(int i = 0; i <= NUM_ROWS; i++) {
        u8g2.drawHLine(START_X, START_Y + (i * ROW_HEIGHT), COL_WIDTH * NUM_COLS);
    }
}

void setup(void) {
    Serial.begin(115200);
    Serial.println(F("VL53L0X Distance Sensor Test"));
    
    u8g2.begin();
    u8g2.setContrast(255);
    Serial.println(F("OLED Display Initialized"));
    
    if (!lox.begin()) {
        Serial.println(F("Failed to boot VL53L0X"));
        u8g2.firstPage();
        do {
            u8g2.setDrawColor(WHITE);
            u8g2.setFont(u8g2_font_ncenB08_tr);
            u8g2.drawStr(0, 10, "Sensor Error!");
        } while (u8g2.nextPage());
        while(1);
    }
    
    Serial.println(F("VL53L0X sensor initialized successfully"));
}

void loop(void) {
    VL53L0X_RangingMeasurementData_t measure;
    lox.rangingTest(&measure, false);
    
    u8g2.firstPage();
    do {
        // Draw table
        u8g2.setDrawColor(GRAY);
        drawTable();
        
        // Set font
        u8g2.setFont(u8g2_font_7x13_tr);
        u8g2.setDrawColor(WHITE);
        
        // Column headers (T1, T2, T3)
        u8g2.drawStr(START_X + COL_WIDTH + 10, START_Y + 15, "T1");
        u8g2.drawStr(START_X + (COL_WIDTH * 2) + 10, START_Y + 15, "T2");
        u8g2.drawStr(START_X + (COL_WIDTH * 3) + 10, START_Y + 15, "T3");
        
        // Row headers (P1, P2, P3)
        u8g2.drawStr(START_X + 5, START_Y + ROW_HEIGHT + 15, "P1");
        u8g2.drawStr(START_X + 5, START_Y + (ROW_HEIGHT * 2) + 15, "P2");
       
        
        if (measure.RangeStatus != 4) {
            float distCm = measure.RangeMilliMeter / 10.0;
            char distStr[10];
            
            // Simulate different distances for each cell
            // In reality, you should use actual sensor values
            float distances[9] = {
                distCm,      // P1-T1
                distCm+10,   // P1-T2
                distCm+20,   // P1-T3
                distCm+30,   // P2-T1
                distCm+40,   // P2-T2
                distCm+50,   // P2-T3
            };
            
            // Print distances in cells
            for(int row = 1; row <= 3; row++) {
                for(int col = 1; col <= 3; col++) {
                    int index = ((row-1) * 3) + (col-1);
                    sprintf(distStr, "%.0f", distances[index]);
                    u8g2.drawStr(
                        START_X + (col * COL_WIDTH) + 5,
                        START_Y + (row * ROW_HEIGHT) + 15,
                        distStr
                    );
                }
            }
            
            Serial.print(F("Distance: "));
            Serial.print(distCm);
            Serial.println(F(" cm"));
        } else {
            // Error message if sensor is out of range
            u8g2.drawStr(START_X, START_Y + (ROW_HEIGHT * 3) + 15, "Out of range");
        }
    } while (u8g2.nextPage());
    
    delay(100);
}
