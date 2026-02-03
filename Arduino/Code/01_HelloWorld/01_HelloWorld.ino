/**
 * 01_HelloWorld - Basic Display Test
 * 
 * Hardware: ESP32-S3 + ST7789 TFT (240x280)
 * 
 * This example demonstrates:
 *   - Initialize SPI display
 *   - Draw text on screen
 *   - Random text position and color
 * 
 * Library: Arduino_GFX_Library
 */

#include <Arduino.h>
#include "Arduino_GFX_Library.h"
#include "pin_config.h"
#include "HWCDC.h"

// USB Serial for debug output
HWCDC USBSerial;

// Create SPI bus and display driver
Arduino_DataBus *bus = new Arduino_ESP32SPI(LCD_DC, LCD_CS, LCD_SCK, LCD_MOSI);
Arduino_GFX *gfx = new Arduino_ST7789(bus, LCD_RST, 0, true, LCD_WIDTH, LCD_HEIGHT, 0, 20, 0, 0);

void setup() {
  // Initialize serial for debugging
  USBSerial.begin(115200);
  USBSerial.println("Hello World Example");

  // Initialize display
  if (!gfx->begin()) {
    USBSerial.println("Display init failed!");
  }
  gfx->fillScreen(BLACK);

  // Turn on backlight
  pinMode(LCD_BL, OUTPUT);
  digitalWrite(LCD_BL, HIGH);

  // Show initial message
  gfx->setCursor(10, 10);
  gfx->setTextColor(RED);
  gfx->println("Hello World!");

  delay(5000);
}

void loop() {
  // Random position
  gfx->setCursor(random(gfx->width()), random(gfx->height()));
  
  // Random foreground and background color
  gfx->setTextColor(random(0xffff), random(0xffff));
  
  // Random text size (scaleX, scaleY, margin)
  gfx->setTextSize(random(6), random(6), random(2));
  
  gfx->println("Hello World!");

  delay(1000);
}
