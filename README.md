# Library for ILI9341-based TFT displays on the Espressif ESP32 native build platform

See end of this file for credits and code provenance

This uses a combination of [Adafruit's Arduino ILI9341 driver](https://github.com/adafruit/Adafruit_ILI9341) (heavily modified) and [their Arduino display graphics library](https://github.com/adafruit/Adafruit-GFX-Library) (with a few specific fonts from there compiled in).

Note that the original Arduino libraries above should work for those using [the Arduino-ESP32 project](https://github.com/espressif/arduino-esp32), with some modificaitons for your particular setup (e.g., the ESP-WROVER-KIT). The ILI9341 driver in particular appears to be ESP32-aware now. While there are no plans to convert the native SDK-based demos below to Arduino-ESP32 code, Arduino-specific demos are here as well.

## Setup
The sample project is to be built with Espressif IDF as configured for the ESP32:
https://github.com/espressif/esp-idf

## ILI9341 driver
The driver itself and needed dependencies are in /driver and /include/driver.
The driver is written in C++ which is not well supported by ESP8266 toolchain and sdk, so some dirty hack is needed to properly contstruct C++ objects. The code for this is in user/routines.cpp.

## SPI stuff
In spite of the fact that according to the datasheet max ILI9341's clock speed is 10MHz mine robustly works at up to 40MHz so I added SPI speed prescaler macro at the beginning of hspi.c.
Defining it to 1 means HSPI will be clocked at 40MHz, 4 means 10 MHz.

## Sample code (tested using the ESP-WROVER-KIT)

  - demos/arduino-esp32 - Install Arduino-ESP32, add the Adafruit_ILI9341 and the Adafruit-GFX-Library, then build and flash via the Arduino IDE.

  - demos/esp-idf - Install Espressif's esp-idf SDK and build and flash using the usual SDK methods.


*** Note: HVAC display demo is the only part being migrated from ESP8266 at this point ***

The sample code consists of two parts:

1) Rotating cube https://youtu.be/kEWThKicO0Q

2) Sample HVAC controller UI (for the extended version of this: http://habrahabr.ru/post/214257/) https://youtu.be/VraLl8XK1CI

What to demonstrate is controlled by macro UIDEMO defined at the beginning of user_main.cpp. If it's defined then the sample HVAC controller UI is shown, else, the rotating cube is rendered.

*** Note: For HVAC display demo we are using the ESP-WROVER-KIT ***

## Wiring
The code uses hardware HSPI with hardware controlled CS, so the wiring shall be as follows: 

    ILI9341 pin --> ESP32 GPIO pin of choice
    MOSI        --> GPIO13
    MISO        --> n/c
    CLK         --> GPIO14
    CS          --> GPIO15
    DC          --> GPIO2
    Reset       --> pull up to 3.3V
    GND         --> ground

## Credits
Basis 1: https://github.com/adafruit/Adafruit_ILI9341
    This is a library for the Adafruit ILI9341 display products
    
    This library works with the Adafruit 2.8" Touch Shield V2 (SPI)
      ----> http://www.adafruit.com/products/1651
     
    Check out the links above for our tutorials and wiring diagrams.
    These displays use SPI to communicate, 4 or 5 pins are required
    to interface (RST is optional).
    
    Adafruit invests time and resources providing this open source code,
    please support Adafruit and open-source hardware by purchasing
    products from Adafruit!

    Written by Limor Fried/Ladyada for Adafruit Industries.
    MIT license, all text above must be included in any redistribution

Basis 2: https://github.com/adafruit/Adafruit-GFX-Library
    This is the core graphics library for all our displays, providing a common set of graphics primitives (points, lines, circles, etc.). It needs to be paired with a hardware-specific library for each display device we carry (to handle the lower-level functions).

    Adafruit invests time and resources providing this open source code, please support Adafruit and open-source hardware by purchasing products from Adafruit!

    Written by Limor Fried/Ladyada for Adafruit Industries.
    BSD license, check license.txt for more information.
    All text above must be included in any redistribution.

Both as modified by Sermus for the ESP8266: https://github.com/Sermus/ESP8266_Adafruit_ILI9341

Further modifications by Rudi, et. al. for the ESP32: http://esp32.com/viewtopic.php?f=18&t=636#p2907

Extended with text rendering routines from: http://www.instructables.com/id/Arduino-TFT-display-and-font-library/?ALLSTEPS

Original hspi code: https://github.com/Perfer/esp8266_ili9341


