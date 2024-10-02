# ESP32-Wardriving
ESP32 wardriving project, forked from Hak5's ESP8266 wardriver. See ESP32 documentation [here](https://docs.espressif.com/projects/esp-idf/en/latest/esp32/get-started/).

Documentation on this page is still in development.

## Components List
| Component | Notes |
| --- | --- |
| ESP32 | The brains of the operation + it can speak WiFi. I went with the ESP-WROOM-32 model |
| GPS Module | Gathers location data. Talks to the ESP32 over serial UART |
| SD Reader + Card | Stores relavant data as a csv that we can upload to WiGLE |


## Hardware Setup
Always double check your pin layouts!! Chances are your boards are slightly different than mine.

**GPS Module** 
| GPS Pin | ESP32 Pin (Purpose)
| --- | --- |
| VCC | 1 (3.3v)
| RX | 28 (TX)
| TX | 27 (RX)
| GND | 32 (GND)

**SD Reader Module** 
| SD Reader Pin | ESP32 Pin (Purpose)
| --- | --- |
| GND | 38 (GND)
| VCC | 19 (5v)
| MISO | 31 (MISO)
| MOSI | 37 (MOSI)
| SCK | 30 (SCK)
| CS | 29 (SS)

note: my SD card reader wasn't working for the longest time. I had to read the Amazon reviews to learn that it actually requres 5v, not the stated 3.3v

