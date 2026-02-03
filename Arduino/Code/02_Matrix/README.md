# 02_ColorTest

Matrix-style falling code rain effect.

## What it does

Creates the iconic "Matrix" digital rain animation with:
- Multiple columns of falling characters
- White head with green fading trail
- Random character flickering
- Variable fall speeds

## Code Flow

```
setup()
  ├── Init display
  ├── Initialize column arrays:
  │   ├── heads[] - vertical position of each column
  │   ├── speeds[] - fall speed (1-2)
  │   └── chars[][] - random ASCII characters

loop()
  ├── For each column:
  │   ├── Draw trail (head + fading tail)
  │   ├── Erase old tail end
  │   ├── Move head down
  │   └── Reset to top if off screen
  └── Delay 50ms
```

## Key Functions

- `getTrailColor()` - Calculate fade color based on distance from head

## Parameters

| Name | Value | Description |
|------|-------|-------------|
| CHAR_SIZE | 8 | Character pixel size |
| TAIL_LEN | 12 | Trail length in characters |

## Hardware

- ESP32-S3
- ST7789 TFT 240x280

## Library

- Arduino_GFX_Library
