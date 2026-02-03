# 03_SimpleWatch

WiFi NTP clock that syncs time from the internet.

## What it does

1. Connect to WiFi network
2. Sync time from NTP server
3. Display digital clock with date and weekday

## Setup

Edit `pin_config.h` before uploading:

```cpp
#define WIFI_SSID     "YOUR_WIFI_SSID"
#define WIFI_PASSWORD "YOUR_WIFI_PASSWORD"
#define GMT_OFFSET    28800   // Your timezone in seconds
```

## Code Flow

```
setup()
  ├── Init display
  ├── Connect to WiFi
  │   └── Show "Connecting WiFi..."
  ├── Configure NTP server
  ├── Wait for time sync
  └── Draw watch face

loop()
  ├── Get current time
  ├── If second changed:
  │   └── updateDisplay()
  │       ├── Draw HH:MM (large)
  │       ├── Draw SS (small)
  │       ├── Draw YYYY-MM-DD
  │       └── Draw weekday
  └── Delay 100ms
```

## Key Functions

- `drawWatchFace()` - Draw static elements (title)
- `updateDisplay()` - Update time, date, weekday

## Display Layout

```
    ┌─────────────────┐
    │   NTP WATCH     │
    │                 │
    │   12:34   56    │
    │   2024-01-15    │
    │      MON        │
    └─────────────────┘
```

## Hardware

- ESP32-S3
- ST7789 TFT 240x280

## Library

- Arduino_GFX_Library
