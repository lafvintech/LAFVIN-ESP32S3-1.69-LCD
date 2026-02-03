/**
 * 04_GFX_WIFIAnalyzer - WiFi Signal Analyzer
 * 
 * Hardware: ESP32-S3 + ST7789 TFT (240x280)
 * 
 * This example demonstrates:
 *   - WiFi network scanning
 *   - Signal strength visualization
 *   - Channel distribution graph
 * 
 * Library: Arduino_GFX_Library
 */

#include <Arduino.h>
#include <WiFi.h>
#include "Arduino_GFX_Library.h"
#include "pin_config.h"
#include "HWCDC.h"

HWCDC USBSerial;

// Display driver
Arduino_DataBus *bus = new Arduino_ESP32SPI(LCD_DC, LCD_CS, LCD_SCK, LCD_MOSI);
Arduino_GFX *gfx = new Arduino_ST7789(bus, LCD_RST, 0, true, LCD_WIDTH, LCD_HEIGHT, 0, 20, 0, 0);

// Layout parameters
int16_t banner_height, graph_baseline, graph_height, channel_width, signal_width;

// RSSI range for mapping
#define RSSI_CEILING -40    // Strongest signal
#define RSSI_FLOOR   -100   // Weakest signal

// Scan interval in milliseconds
#define SCAN_INTERVAL 2000

// Color for each channel (1-14)
uint16_t channel_color[] = {
  RED, ORANGE, YELLOW, GREEN, CYAN, BLUE, MAGENTA,
  RED, ORANGE, YELLOW, GREEN, CYAN, BLUE, MAGENTA
};

void setup() {
  USBSerial.begin(115200);
  USBSerial.println("WiFi Analyzer started!");
  
  // Backlight on
  pinMode(LCD_BL, OUTPUT);
  digitalWrite(LCD_BL, HIGH);
  
  // Init display
  gfx->begin();
  gfx->fillScreen(BLACK);
  
  // Calculate layout
  banner_height = 24;
  graph_baseline = LCD_HEIGHT - 20;
  graph_height = graph_baseline - banner_height - 30;
  channel_width = LCD_WIDTH / 17;
  signal_width = channel_width * 2;
  
  // Draw title
  gfx->setTextSize(2);
  gfx->setTextColor(CYAN);
  gfx->setCursor(30, 0);
  gfx->print("WiFi Analyzer");
  
  // Set WiFi to station mode (scan only)
  WiFi.mode(WIFI_STA);
  WiFi.disconnect();
  delay(100);
}

void loop() {
  // Scan for networks
  int n = WiFi.scanNetworks(false, true, true, 500);
  
  // Clear graph area
  gfx->fillRect(0, banner_height, LCD_WIDTH, LCD_HEIGHT - banner_height, BLACK);
  gfx->setTextSize(1);
  
  if (n == 0) {
    gfx->setTextColor(WHITE);
    gfx->setCursor(30, banner_height + 10);
    gfx->println("No networks found");
  } else {
    // Statistics arrays
    uint8_t ap_count[14] = {0};
    int32_t peak_rssi[14];
    int16_t peak_id[14];
    
    for (int i = 0; i < 14; i++) {
      peak_rssi[i] = RSSI_FLOOR;
      peak_id[i] = -1;
    }
    
    // First pass: count APs and find strongest signal per channel
    for (int i = 0; i < n; i++) {
      int ch = WiFi.channel(i);
      if (ch < 1 || ch > 14) continue;
      
      int idx = ch - 1;
      int32_t rssi = WiFi.RSSI(i);
      
      ap_count[idx]++;
      if (rssi > peak_rssi[idx]) {
        peak_rssi[idx] = rssi;
        peak_id[idx] = i;
      }
    }
    
    // Second pass: draw signal arcs
    for (int i = 0; i < n; i++) {
      int ch = WiFi.channel(i);
      if (ch < 1 || ch > 14) continue;
      
      int idx = ch - 1;
      int32_t rssi = WiFi.RSSI(i);
      uint16_t color = channel_color[idx];
      
      // Map RSSI to height
      int height = constrain(map(rssi, RSSI_FLOOR, RSSI_CEILING, 1, graph_height), 1, graph_height);
      int offset = (ch + 1) * channel_width;
      
      // Draw arc
      gfx->startWrite();
      gfx->writeEllipseHelper(offset, graph_baseline + 1, signal_width, height, 0b0011, color);
      gfx->endWrite();
      
      // Show SSID for strongest signal on each channel
      if (i == peak_id[idx]) {
        String ssid = WiFi.SSID(i);
        if (ssid.length() == 0) ssid = "Hidden";
        if (ssid.length() > 10) ssid = ssid.substring(0, 10) + "..";
        
        gfx->setTextColor(color);
        gfx->setCursor(offset - signal_width, graph_baseline - height - 10);
        gfx->printf("%s(%d)", ssid.c_str(), rssi);
      }
    }
    
    // Show network count
    gfx->setTextColor(WHITE);
    gfx->setCursor(30, banner_height);
    gfx->printf("%d networks found", n);
  }
  
  // Draw X-axis and channel labels
  gfx->drawFastHLine(0, graph_baseline, LCD_WIDTH, WHITE);
  for (int ch = 1; ch <= 14; ch++) {
    int offset = (ch + 1) * channel_width;
    gfx->setTextColor(channel_color[ch - 1]);
    gfx->setCursor(offset - 3, graph_baseline + 2);
    gfx->print(ch);
  }
  
  delay(SCAN_INTERVAL);
}
