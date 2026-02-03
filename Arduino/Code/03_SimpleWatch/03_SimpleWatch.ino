/**
 * 03_SimpleWatch - WiFi NTP Clock
 * 
 * Hardware: ESP32-S3 + ST7789 TFT (240x280)
 * 
 * This example demonstrates:
 *   - WiFi connection
 *   - NTP time synchronization
 *   - Digital clock display with date
 * 
 * Before use: Edit WiFi settings in pin_config.h
 * 
 * Library: Arduino_GFX_Library
 */

#include <Arduino.h>
#include <WiFi.h>
#include <time.h>
#include "Arduino_GFX_Library.h"
#include "pin_config.h"
#include "HWCDC.h"

HWCDC USBSerial;

// Display driver
Arduino_DataBus *bus = new Arduino_ESP32SPI(LCD_DC, LCD_CS, LCD_SCK, LCD_MOSI);
Arduino_GFX *gfx = new Arduino_ST7789(bus, LCD_RST, 0, true, LCD_WIDTH, LCD_HEIGHT, 0, 20, 0, 0);

bool timeSync = false;
int lastSecond = -1;

void setup() {
  USBSerial.begin(115200);
  USBSerial.println("SimpleWatch NTP Demo");
  
  // Backlight on
  pinMode(LCD_BL, OUTPUT);
  digitalWrite(LCD_BL, HIGH);
  
  // Init display
  gfx->begin();
  gfx->fillScreen(BLACK);
  
  // Show connecting status
  gfx->setTextColor(WHITE);
  gfx->setTextSize(2);
  gfx->setCursor(20, 100);
  gfx->println("Connecting WiFi...");
  
  // Connect to WiFi
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  
  int retry = 0;
  while (WiFi.status() != WL_CONNECTED && retry < 30) {
    delay(500);
    USBSerial.print(".");
    retry++;
  }
  
  if (WiFi.status() == WL_CONNECTED) {
    USBSerial.println("\nWiFi Connected!");
    USBSerial.println(WiFi.localIP());
    
    gfx->fillScreen(BLACK);
    gfx->setCursor(20, 100);
    gfx->println("WiFi OK!");
    gfx->setCursor(20, 130);
    gfx->println("Syncing time...");
    
    // Configure NTP server
    configTime(GMT_OFFSET, DST_OFFSET, NTP_SERVER);
    
    // Wait for time sync
    struct tm timeinfo;
    retry = 0;
    while (!getLocalTime(&timeinfo) && retry < 10) {
      delay(500);
      retry++;
    }
    
    if (getLocalTime(&timeinfo)) {
      timeSync = true;
      USBSerial.println("Time synced!");
    }
  } else {
    gfx->fillScreen(BLACK);
    gfx->setTextColor(RED);
    gfx->setCursor(20, 100);
    gfx->println("WiFi Failed!");
  }
  
  gfx->fillScreen(BLACK);
  drawWatchFace();
}

void loop() {
  if (timeSync) {
    struct tm timeinfo;
    if (getLocalTime(&timeinfo)) {
      // Update display only when second changes
      if (timeinfo.tm_sec != lastSecond) {
        lastSecond = timeinfo.tm_sec;
        updateDisplay(&timeinfo);
      }
    }
  }
  delay(100);
}

/**
 * Draw static watch face elements
 */
void drawWatchFace() {
  gfx->setTextColor(CYAN);
  gfx->setTextSize(2);
  gfx->setCursor(65, 60);
  gfx->println("NTP WATCH");
}

/**
 * Update time display
 */
void updateDisplay(struct tm *timeinfo) {
  // Clear time area
  gfx->fillRect(30, 110, 190, 90, BLACK);
  
  // Time HH:MM (large)
  gfx->setTextColor(WHITE);
  gfx->setTextSize(4);
  gfx->setCursor(45, 115);
  gfx->printf("%02d:%02d", timeinfo->tm_hour, timeinfo->tm_min);
  
  // Seconds (smaller, to the right)
  gfx->setTextSize(3);
  gfx->setCursor(175, 120);
  gfx->printf("%02d", timeinfo->tm_sec);
  
  // Date YYYY-MM-DD
  gfx->setTextColor(GREEN);
  gfx->setTextSize(2);
  gfx->setCursor(60, 165);
  gfx->printf("%04d-%02d-%02d", 
    timeinfo->tm_year + 1900, 
    timeinfo->tm_mon + 1, 
    timeinfo->tm_mday);
  
  // Weekday
  const char* weekdays[] = {"SUN", "MON", "TUE", "WED", "THU", "FRI", "SAT"};
  gfx->setTextColor(YELLOW);
  gfx->setCursor(100, 195);
  gfx->println(weekdays[timeinfo->tm_wday]);
}
