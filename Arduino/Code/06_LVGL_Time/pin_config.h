/**
 * pin_config.h - Hardware Pin Definitions
 * 
 * Board: ESP32-S3 + ST7789 TFT (240x280)
 */

#ifndef PIN_CONFIG_H
#define PIN_CONFIG_H

// Display size
#define LCD_WIDTH   240
#define LCD_HEIGHT  280

// SPI pins for ST7789
#define LCD_MOSI    39    // Data out
#define LCD_SCK     38    // Clock
#define LCD_CS      14    // Chip select
#define LCD_DC      10    // Data/Command
#define LCD_RST     3     // Reset
#define LCD_BL      13    // Backlight

// ==================== WiFi Settings ====================
// Change these to your WiFi credentials
#define WIFI_SSID     "2A811"
#define WIFI_PASSWORD "la1234567890"

// ==================== NTP Settings ====================
#define NTP_SERVER    "pool.ntp.org"
#define GMT_OFFSET    28800   // UTC+8 (seconds), change for your timezone
#define DST_OFFSET    0       // Daylight saving time offset

#endif
