# NTP GPS Firmware
This firmware is specific to the device described in the Hardware section. It is a simple Arduino sketch for the ESP32. The only required library is TinyGPSPlus, included in the libraries folder, though this copy does not track upstream and may be (and is likely) very out of date.

## Use one of the compiled binaries
Compiled binaries are available if you don't want to or are unable to use Arduino. They would be flashed to the board using esptool.

## Set up Arduino to work with ESP32
- Open the Arduino preferences pane, and add an `Additional Boards Manager URL` of `https://raw.githubusercontent.com/espressif/arduino-esp32/gh-pages/package_esp32_index.json`.
- Open `Tools -> Board -> Boards Manager`, search for "ESP32", and install the `esp32` package by Espressif Systems.
- Select the ESP32 parameters from the Tools Menu:
    - Board: ESP32 Dev Module
    - Flash Size: 4MB
    - PSRAM: Disabled
    - Port: <your_device_port>