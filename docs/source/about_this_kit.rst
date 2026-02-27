.. _about-this-kit:

=====================
About This Kit
=====================

.. .. thumbnail:: ./img/other/main_product.png
   :align: center
   :width: 600px

Product Overview
=====================

The LAFVIN ESP32S3 1.69 LCD is a compact development board built around the ESP32-S3 microcontroller, featuring a 1.69-inch LCD display, a detachable OV3660 camera module, and onboard RGB LED. It is designed for developers and learners who want to explore AI voice assistants, embedded GUI development, computer vision, and multi-platform programming with Arduino and ESP-IDF.

The kit comes with a complete set of tutorials covering Xiaozhi AI, LVGL graphics, OpenCV image processing, and YOLO object detection, making it a versatile platform for hands-on embedded and AI learning.

Hardware Specifications
========================

Core Module
-----------

- Chip: ESP32-S3-N16R8
- Flash: 16MB
- PSRAM: 8MB
- Processor: Dual-core Xtensa LX7, up to 240MHz
- Wireless: Wi-Fi 802.11 b/g/n + Bluetooth 5.0 (LE)

Display
-----------

- Screen: 1.69-inch TFT LCD
- Driver: ST7789
- Resolution: 240 x 280 pixels
- Interface: SPI
- Backlight: Software-adjustable

Camera
-----------

- Sensor: OV3660
- Connection: Detachable module (FPC connector)
- Supports UVC protocol for use as a USB webcam

Buttons and LED
-----------------

- Boot button (GPIO0)
- Reset button (RST)
- Power switch (for battery operation)
- WS2812 addressable RGB LED (on the back)

Interfaces
-----------

- USB Type-C (power and data)
- SH1.0-2P battery connector
- SH1.0-2P speaker connector

Physical Dimensions
--------------------

- Size: approx. 31mm x 38mm

What's in the Box
=====================

.. list-table::
   :widths: 10 50 20
   :header-rows: 1

   * - #
     - Item
     - Quantity
   * - 1
     - LAFVIN ESP32S3 1.69 LCD Development Board
     - 1
   * - 2
     - OV3660 Camera Module
     - 1
   * - 3
     - Speaker
     - 1
   * - 4
     - Type-C USB Cable
     - 1


Tutorials Overview
=====================

This kit includes tutorials for multiple development platforms and application areas:

.. list-table::
   :widths: 30 70
   :header-rows: 1

   * - Chapter
     - Description
   * - :ref:`Xiaozhi AI <xiaozhi_ai>`
     - Set up an AI voice assistant using the Xiaozhi open-source firmware
   * - :ref:`Arduino <arduino_tutorial>`
     - Display demos, GIF player, WiFi image viewer, LVGL watch face, and LED control
   * - :ref:`ESP-IDF <esp_idf>`
     - Low-level development with hello world, blink, USB webcam, and Xiaozhi project
   * - :ref:`Advanced <advance_tutorial>`
     - Python-based projects using OpenCV and YOLO with the UVC camera

.. note::
   Some tutorials require additional software installation (Arduino IDE, ESP-IDF, Python). Please refer to the :ref:`Appendix <appendix>` for setup instructions.
