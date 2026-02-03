# 04_GFX_WIFIAnalyzer

WiFi signal analyzer with graphical display.

## What it does

1. Scan nearby WiFi networks every 2 seconds
2. Display signal strength as arcs on a channel graph
3. Show SSID and RSSI for strongest network on each channel
4. Color-coded by channel number

## Code Flow

```
setup()
  ├── Init display
  ├── Calculate layout parameters
  ├── Draw title
  └── Set WiFi to scan mode

loop()
  ├── Scan WiFi networks
  ├── Clear graph area
  ├── First pass: find strongest signal per channel
  ├── Second pass: draw signal arcs
  │   ├── Map RSSI to arc height
  │   ├── Draw arc with channel color
  │   └── Show SSID for peak signal
  ├── Draw X-axis with channel labels
  └── Wait 2 seconds
```

## Display Layout

```
┌─────────────────────┐
│   WiFi Analyzer     │
│ 8 networks found    │
│                     │
│  SSID1    SSID2     │  <- SSID labels
│   ╭╮       ╭╮       │  <- Signal arcs
│  ╭╯╰╮     ╭╯╰╮      │
│ ─┴──┴─────┴──┴───── │  <- Baseline
│  1 2 3 4 5 6 7 ...  │  <- Channel numbers
└─────────────────────┘
```

## Parameters

| Name | Value | Description |
|------|-------|-------------|
| RSSI_CEILING | -40 | Strongest signal (dBm) |
| RSSI_FLOOR | -100 | Weakest signal (dBm) |
| SCAN_INTERVAL | 2000 | Scan interval (ms) |

## Hardware

- ESP32-S3
- ST7789 TFT 240x280

## Library

- Arduino_GFX_Library
