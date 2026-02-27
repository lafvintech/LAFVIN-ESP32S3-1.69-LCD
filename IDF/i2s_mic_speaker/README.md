# I2S Mic & Speaker Echo Demo

## Overview

Real-time audio echo (loopback): records from ICS-43434 microphone and plays back through MAX98357 speaker.

## Hardware

- ESP32-S3 N16R8
- ICS-43434 I2S MEMS Microphone
- MAX98357 I2S DAC Amplifier

### Pin Connections

| Device   | Signal | GPIO |
|----------|--------|------|
| ICS-43434| WS     | 40   |
| ICS-43434| SCK    | 41   |
| ICS-43434| SD     | 42   |
| MAX98357 | DIN    | 45   |
| MAX98357 | BCLK   | 46   |
| MAX98357 | LRCLK  | 47   |

## Build & Flash

```bash
idf.py set-target esp32s3
idf.py build
idf.py -p PORT flash monitor
```

## Notes

- Sample rate: 16kHz, 32-bit RX (24-bit valid) → 16-bit TX, mono
- A simple volume boost (12dB) is applied to compensate for the ICS-43434's low output level
- Adjust the shift value in `echo_task()` if the volume is too loud or too quiet
