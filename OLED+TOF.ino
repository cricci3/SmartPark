#include <Arduino.h>
#include <U8g2lib.h>
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

void setup(void) {
  // Initialize Serial communication
  Serial.begin(115200);
  Serial.println(F("VL53L0X Distance Sensor Test"));
  
  // Initialize OLED display
  u8g2.begin();
  Serial.println(F("OLED Display Initialized"));
  
  // Initialize VL53L0X sensor
  if (!lox.begin()) {
    Serial.println(F("Failed to boot VL53L0X"));
    // Display error message on OLED
    u8g2.firstPage();
    do {
      u8g2.setFont(u8g2_font_ncenB08_tr);
      u8g2.drawStr(0, 10, "Sensor Error!");
    } while (u8g2.nextPage());
    while(1);
  }
  Serial.println(F("VL53L0X sensor initialized successfully"));
}

void loop(void) {
  VL53L0X_RangingMeasurementData_t measure;
  
  // Get distance measurement
  lox.rangingTest(&measure, false);
  
  // Update display
  u8g2.firstPage();
  do {
    u8g2.setFont(u8g2_font_ncenB14_tr);  // Using larger font for better visibility
    
    // Display title
    u8g2.drawStr(0, 20, "Distance:");
    
    // Create buffer for distance value
    char distStr[32];
    
    if (measure.RangeStatus != 4) {
      // Calculate distance in cm
      float distCm = measure.RangeMilliMeter / 10.0;
      
      // Display distance in cm on OLED with larger font
      sprintf(distStr, "%.1f cm", distCm);
      u8g2.drawStr(0, 50, distStr);
      
      // Output to Serial Monitor
      Serial.print(F("Distance: "));
      Serial.print(distCm);
      Serial.println(F(" cm"));
      
    } else {
      // Display out of range message on OLED
      u8g2.drawStr(0, 50, "Out of range!");
      // Output to Serial Monitor
      Serial.println(F("Out of range!"));
    }
    
  } while (u8g2.nextPage());
  
  delay(100); // Small delay between measurements
}
