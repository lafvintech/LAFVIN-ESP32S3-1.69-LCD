- [中文](https://github.com/DFRobot/DFR1154_Examples/blob/master/6.2%20yolo/README_CN.md)

This example demonstrates object detection and pose estimation using YOLO models on the computer side.

## Quick Start

### Windows

```bash
git clone <repo-url>
cd Advance/2.yolo/Computer program
setup.bat
```

### Linux / macOS

```bash
git clone <repo-url>
cd Advance/2.yolo/Computer\ program
chmod +x setup.sh
./setup.sh
```

The script will automatically create a virtual environment and install all dependencies. Model files will be downloaded automatically on first run.

### Run Demos

```bash
# Activate virtual environment first
# Windows: venv\Scripts\activate
# Linux/Mac: source venv/bin/activate

# Object detection (YOLOv5s, 80 classes)
python 1.yolo_object_recognition.py

# Pose estimation (YOLOv8n-pose, 17 keypoints)
python 2.yolo_pose.py
```

## Prerequisites

1. Burn the code to the module (use the code from Section 5.3 or 5.4).
2. Python 3.8+ installed ([python.org](https://www.python.org/)).

## Controls

- Press `q` to quit
- Close the window via X button to quit

## Notes

- Model files (~6-14MB) are downloaded automatically on first run.
- Use `-c` flag to specify camera index: `python 1.yolo_object_recognition.py -c 1`
