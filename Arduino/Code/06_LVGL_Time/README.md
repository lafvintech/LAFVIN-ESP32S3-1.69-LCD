# 06_LVGL_Time

Modern watch face using LVGL graphics library with NTP time sync.

## What it does

1. Connect to WiFi and sync time from NTP server
2. Display digital clock with animated seconds arc
3. Show date and weekday
4. Modern dark theme UI

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
  ├── Init LVGL
  │   ├── Create display buffer
  │   └── Register display driver
  ├── createWatchFace()
  │   ├── Create outer arc (decoration)
  │   ├── Create seconds arc (animated)
  │   ├── Create time label
  │   ├── Create date label
  │   ├── Create weekday label
  │   └── Create status label
  ├── Connect WiFi
  ├── Sync NTP time
  └── Create update timer (500ms)

loop()
  └── lv_timer_handler()
      └── updateTimeCallback()
          └── updateTimeDisplay()
```

## UI Layout

```
┌─────────────────────┐
│    ╭───────────╮    │  <- Outer arc
│   ╭─────────────╮   │  <- Seconds arc
│   │             │   │
│   │   12:34     │   │  <- Time (large)
│   │  2024/01/15 │   │  <- Date
│   │   Monday    │   │  <- Weekday
│   ╰─────────────╯   │
│    NTP Synced       │  <- Status
└─────────────────────┘
```

## LVGL Widgets Used

- `lv_arc` - Circular progress indicator
- `lv_label` - Text display
- `lv_timer` - Periodic callback

## Hardware

- ESP32-S3
- ST7789 TFT 240x280

## Libraries

- Arduino_GFX_Library
- lvgl (8.x)
