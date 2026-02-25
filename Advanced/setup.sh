#!/bin/bash
echo "========================================"
echo "  Advance Demo - Auto Setup (Linux/Mac)"
echo "========================================"
echo

# Check Python
if ! command -v python3 &> /dev/null; then
    echo "[ERROR] Python3 not found. Please install Python first."
    exit 1
fi

# Create virtual environment
if [ ! -d "venv" ]; then
    echo "[1/2] Creating virtual environment..."
    python3 -m venv venv
else
    echo "[1/2] Virtual environment already exists, skipping..."
fi

# Activate and install dependencies
echo "[2/2] Installing dependencies (this may take a few minutes)..."
source venv/bin/activate
pip install -r requirements.txt

echo
echo "========================================"
echo "  Setup complete!"
echo "========================================"
echo
echo "Activate the environment:"
echo "  source venv/bin/activate"
echo
echo "Then run any demo:"
echo "  python 1.openCV/openCV.py"
echo "  python 1.openCV/color_tracker.py"
echo "  python 1.openCV/face_detector.py"
echo "  python 2.yolo/Computer\ program/1.yolo_object_recognition.py"
echo "  python 2.yolo/Computer\ program/2.yolo_pose.py"
