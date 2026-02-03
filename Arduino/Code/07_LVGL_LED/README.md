# 07_LVGL_LED

LVGL interface with WS2812 RGB LED control using button interactions.

## Features

- Single click GPIO0 button to change LED to random color
- Double click GPIO0 button to toggle LED on/off
- LVGL UI displays current LED status and hex color value
- Visual LED indicator with glow effect

## Code Flow

1. **Setup**: Initialize display, LVGL, WS2812 LED, and button
2. **Create UI**: Build dark-themed interface with circular LED indicator, status label, and color value
3. **Main Loop**: 
   - Poll button state with debounce logic
   - Detect single vs double click (300ms timeout)
   - Update LED hardware and UI on state change
   - Run LVGL timer handler

## Hardware

- ESP32-S3
- ST7789 TFT 240x280
- WS2812 RGB LED on GPIO48
- Button on GPIO0

## Dependencies

- Arduino_GFX_Library
- lvgl (8.x)
- Adafruit_NeoPixel
