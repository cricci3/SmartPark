#include <U8g2lib.h>
#include <Wire.h>
#include "Adafruit_VL53L0X.h"

// Display setup
U8G2_SSD1327_EA_W128128_F_4W_SW_SPI display(U8G2_R0, 13, 5, 10, 7, 8);
Adafruit_VL53L0X distanceSensor = Adafruit_VL53L0X();

// Display settings
const uint8_t DISPLAY_COLOR_WHITE = 255;
const uint8_t DISPLAY_COLOR_GRAY = 150;

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
// 'handicap', 16x16px
const unsigned char epd_bitmap_handicap [] PROGMEM = {
	0x01, 0x80, 0xfe, 0x7f, 0x3e, 0x7f, 0xde, 0x7e, 0xde, 0x7e, 0x3e, 0x7f, 0x3e, 0x7f, 0xbe, 0x78, 
	0x8e, 0x78, 0xb6, 0x77, 0x36, 0x74, 0xf6, 0x6a, 0xf6, 0x6a, 0x0e, 0x67, 0xfe, 0x7f, 0x01, 0x80
};
// 'power', 16x16px
const unsigned char epd_bitmap_power [] PROGMEM = {
	0xfc, 0x3f, 0xe0, 0x07, 0xef, 0xf7, 0xef, 0xf7, 0xef, 0xf7, 0xef, 0xf7, 0xee, 0xf7, 0xee, 0xf7, 
	0xec, 0x37, 0xef, 0x77, 0xef, 0x77, 0xef, 0xf7, 0xef, 0xf7, 0xef, 0xf7, 0xef, 0xf7, 0xe0, 0x0f
};
const unsigned char epd_bitmap_disabled_sign [] PROGMEM = {
	0x9f, 0xff, 0x1f, 0xff, 0xbf, 0xff, 0xbf, 0xff, 0xbf, 0xff, 0xbf, 0xff, 0x3f, 0xf8, 0xb3, 0xff, 
	0xbb, 0xff, 0x39, 0xf8, 0xfd, 0xf7, 0xfd, 0xf7, 0xfb, 0xc7, 0xfb, 0xcf, 0xe7, 0xfc, 0x0f, 0xfe
};

// Funzione per disegnare un'icona 16x16 in una cella
void drawIcon16x16(uint8_t row, uint8_t col, const unsigned char* iconBitmap) {
    // Calcola posizione della cella
    int cellX = grid.startX + (col * grid.cellWidth);
    int cellY = grid.startY + (row * grid.cellHeight);
    
    // Centra l'icona nella cella (16x16)
    int iconX = cellX + 7;  // (31 - 16) / 2 ≈ 7
    int iconY = cellY + 7;
    
    // Disegna l'icona
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

void drawText(uint8_t row, uint8_t col, const char* text, int offsetX = 5, int offsetY = 15) {
    int x = grid.startX + (col * grid.cellWidth) + offsetX;
    int y = grid.startY + (row * grid.cellHeight) + offsetY;
    display.drawStr(x, y, text);
}

void drawCenteredText(uint8_t row, uint8_t col, const char* text) {
    int cellX = grid.startX + (col * grid.cellWidth);
    int cellY = grid.startY + (row * grid.cellHeight);
    
    int textWidth = display.getStrWidth(text);
    int x = cellX + (grid.cellWidth - textWidth) / 2;
    int y = cellY + grid.cellHeight/2 + 5;
    display.drawStr(x, y, text);
}

void drawGrid() {
    display.setDrawColor(DISPLAY_COLOR_GRAY);
    
    // Linee verticali
    for(uint8_t i = 0; i <= grid.columns; i++) {
        int x = grid.startX + (i * grid.cellWidth);
        display.drawVLine(x, grid.startY, grid.cellHeight * grid.rows);
    }
    
    // Linee orizzontali
    for(uint8_t i = 0; i <= grid.rows; i++) {
        int y = grid.startY + (i * grid.cellHeight);
        display.drawHLine(grid.startX, y, grid.cellWidth * grid.columns);
    }
}

void updateDistances(float baseDistance) {
    char distStr[10];
    const float DISTANCE_INCREMENT = 10.0;
    
    for(uint8_t row = 1; row <= 2; row++) {
        for(uint8_t col = 1; col <= 3; col++) {
            float distance = baseDistance + ((row-1)*3 + (col-1)) * DISTANCE_INCREMENT;
            sprintf(distStr, "%.0f", distance);
            drawCenteredText(row, col, distStr);
        }
    }
}

void displayError(const char* message) {
    display.firstPage();
    do {
        display.setDrawColor(DISPLAY_COLOR_WHITE);
        display.setFont(u8g2_font_ncenB08_tr);
        display.drawStr(0, 10, message);
    } while(display.nextPage());
}

void updateDisplay() {
    VL53L0X_RangingMeasurementData_t measure;
    distanceSensor.rangingTest(&measure, false);

    display.firstPage();
    do {
        drawGrid();
        
        display.setFont(u8g2_font_7x13_tr);
        display.setDrawColor(DISPLAY_COLOR_WHITE);

        // Headers
        drawIcon16x16(0, 1, epd_bitmap_parking); 
        drawIcon16x16(0, 2, epd_bitmap_disabled_sign); 
        drawIcon16x16(0, 3, epd_bitmap_power); 
        drawText(1, 0, "P1");
        drawText(2, 0, "P2");

        if(measure.RangeStatus != 4) {
            float distCm = measure.RangeMilliMeter / 10.0;
            updateDistances(distCm);
            
            Serial.print(F("Distance: "));
            Serial.print(distCm);
            Serial.println(F(" cm"));
        } else {
            drawText(3, 0, "Out of range");
        }

    } while(display.nextPage());
}

void setup() {
    Serial.begin(115200);
    
    display.begin();
    display.setContrast(255);
    
    if(!distanceSensor.begin()) {
        displayError("Sensor Error!");
        while(1);
    }
}

void loop() {
    updateDisplay();
    delay(100);
}
