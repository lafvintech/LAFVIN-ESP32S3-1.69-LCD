# 01_HelloWorld

Basic display test for ESP32-S3 with ST7789 TFT screen.

## What it does

1. Initialize the SPI display
2. Show "Hello World!" in red
3. Every second, display "Hello World!" at random position with random color and size

## Code Flow

```
setup()
  ├── Init USB Serial (for debug)
  ├── Init ST7789 display via SPI
  ├── Turn on backlight
  └── Show initial "Hello World!"

loop()
  ├── Set random cursor position
  ├── Set random text color
  ├── Set random text size
  └── Print "Hello World!"
```

## Hardware

- ESP32-S3
- ST7789 TFT 240x280
- SPI connection (see pin_config.h)

## Library

- Arduino_GFX_Library
