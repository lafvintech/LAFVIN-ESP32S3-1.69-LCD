# Hello World - ST7789 Display Demo

| Supported Targets | ESP32-S3 |
| ----------------- | -------- |

## Overview

This example initializes an ST7789 TFT display (240x280, SPI interface) and shows chip information on screen, including:

- Chip model and CPU core count
- WiFi / BT / BLE feature flags
- Silicon revision
- Flash size and type
- Minimum free heap size

Chip info is also printed to the serial console.

## Hardware

- ESP32-S3 development board with ST7789 240x280 TFT
- Pin connections:

| Signal | GPIO |
|--------|------|
| MOSI   | 39   |
| SCLK   | 38   |
| CS     | 14   |
| DC     | 10   |
| RST    | 3    |
| BL     | 13   |

## How to Use

```bash
idf.py set-target esp32s3
idf.py build
idf.py -p PORT flash monitor
```

## Project Structure

```
├── CMakeLists.txt
├── main
│   ├── CMakeLists.txt
│   └── hello_world_main.c    Main source: LCD init + chip info display
├── README.md
└── sdkconfig
```
