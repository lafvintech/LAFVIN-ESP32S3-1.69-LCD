/**
 * pin_config.h - Pin Configuration
 * ESP32-S3 + ST7789 (240x280)
 */

#ifndef PIN_CONFIG_H
#define PIN_CONFIG_H

// LCD pin definitions
#define LCD_BL    13
#define LCD_MOSI  39
#define LCD_SCK   38
#define LCD_DC    10
#define LCD_RST   3
#define LCD_CS    14

// LCD dimensions
#define LCD_WIDTH  240
#define LCD_HEIGHT 280

// Button pin
#define BTN_PIN   0

// WS2812 RGB LED
#define LED_PIN   48
#define LED_COUNT 1

// Color definitions
#define BLACK   0x0000
#define WHITE   0xFFFF
#define RED     0xF800
#define GREEN   0x07E0
#define BLUE    0x001F
#define CYAN    0x07FF
#define MAGENTA 0xF81F
#define YELLOW  0xFFE0

#endif
