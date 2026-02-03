# 08_WiFi_ImageViewer

WiFi-based image upload and display. Upload images from phone/computer via web browser.

## Features

- Creates WiFi AP hotspot (Image_Viewer)
- Web interface for image upload
- Client-side image resize to fit 240x280 screen
- JPEG decode and centered display
- New uploads replace previous image

## Code Flow

1. **Setup**: Initialize display, FFat filesystem, WiFi AP, and web server
2. **Show Instructions**: Display WiFi name, password, and IP address
3. **Web Server**: 
   - Serve upload page with JavaScript image processing
   - Browser resizes image before upload (maintains aspect ratio)
   - Save uploaded JPEG to FFat filesystem
4. **Display**: Decode JPEG and center on screen

## Usage

1. Connect to WiFi: `Image_Viewer` (password: `12345678`)
2. Open browser: `192.168.4.1`
3. Select image file (JPG, PNG supported)
4. Image auto-resizes and uploads

## Hardware

- ESP32-S3
- ST7789 TFT 240x280

## Dependencies

- Arduino_GFX_Library
- JPEGDEC

## Partition Table

Requires `app3M_fat9M_16MB` for FFat storage.
