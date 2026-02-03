# 05_GIF_Player

Animated GIF player with WiFi upload capability.

## What it does

1. Create WiFi hotspot (AP mode)
2. Serve web page for GIF upload
3. Save uploaded GIF to flash storage (FFat)
4. Decode and play GIF animation in loop

## Usage

1. Upload sketch to ESP32-S3
2. Connect phone to WiFi: `GIF_Player` (password: `12345678`)
3. Open browser: `192.168.4.1`
4. Upload a GIF file
5. GIF plays automatically

## Code Flow

```
setup()
  ├── Init display
  ├── Init FFat filesystem
  ├── Create WiFi AP hotspot
  ├── Start web server
  ├── Init GIF decoder
  └── Show waiting screen (if no GIF)

loop()
  ├── Handle web requests
  ├── If uploading: skip playback
  └── If hasGif:
      ├── Open GIF file
      ├── Play frames in loop
      │   └── Handle web requests between frames
      └── Close GIF
```

## Key Components

### GIF Decoder Callbacks
- `GIFOpenFile()` - Open file from FFat
- `GIFCloseFile()` - Close file
- `GIFReadFile()` - Read data
- `GIFSeekFile()` - Seek position
- `GIFDraw()` - Render frame line to display

### Web Server
- `/` - Upload page
- `/upload` - Handle file upload

## Partition Table

Requires FFat partition. Use: `app3M_fat9M_16MB`

## Hardware

- ESP32-S3
- ST7789 TFT 240x280

## Libraries

- Arduino_GFX_Library
- AnimatedGIF
