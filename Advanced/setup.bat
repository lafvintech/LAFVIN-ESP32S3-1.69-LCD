@echo off
echo ========================================
echo   Advance Demo - Auto Setup (Windows)
echo ========================================
echo.

REM Check Python
python --version >nul 2>&1
if %errorlevel% neq 0 (
    echo [ERROR] Python not found. Please install Python first: https://www.python.org/
    pause
    exit /b 1
)

REM Create virtual environment
if not exist "venv\Scripts\pip.exe" (
    echo [1/2] Creating virtual environment...
    if exist "venv" rmdir /s /q venv
    python -m venv venv
) else (
    echo [1/2] Virtual environment already exists, skipping...
)

REM Install dependencies using full path (works in both CMD and PowerShell)
echo [2/2] Installing dependencies (this may take a few minutes)...
.\venv\Scripts\pip.exe install -r requirements.txt

echo.
echo ========================================
echo   Setup complete!
echo ========================================
echo.
echo Run any demo:
echo   .\venv\Scripts\python.exe 1.openCV\openCV.py
echo   .\venv\Scripts\python.exe 1.openCV\color_tracker.py
echo   .\venv\Scripts\python.exe 1.openCV\face_detector.py
echo   .\venv\Scripts\python.exe 2.yolo\1.yolo_object_recognition.py
echo   .\venv\Scripts\python.exe 2.yolo\2.yolo_pose.py
echo.
pause
