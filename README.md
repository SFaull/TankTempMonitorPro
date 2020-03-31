# TankTempMonitorPro
Firmware for ESP32 with 5 temperature sensor and LCD for monitoring temperature of water tank

# Parts
* ESP32-WROOM-32
* 1.3" 240x240 ST7789 Display
* 5 off Dallas DS18B20 Onewire Temperature sennsors
* LDR
* Pushbutton

# Pinout

| ESP32         | Peripheral
| ------------- |:-----------------:|
| GND           | All - GND         |
| 3V3           | All - VCC         |
| D2            | Display - DC      |
| D4            | Display - RES     |
| RX2           | DS18B20 - Yellow  |
| D18           | Display - SCL     |
| D19           | Display - BLK     |
| D23           | Display - SDA     |
| D14           | Push Button       |
| D33           | LDR Output        |
