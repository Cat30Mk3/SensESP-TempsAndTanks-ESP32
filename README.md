# SensESP Temps and Tanks

This repository contains the current source code for the data collection and NMEA2000 message publishing for our sailboat Avalon.

primary features of this program include:
- reading the tank levels of up to 4 tanks using either capacitive level measurement or resistive float measurement
- reading of three DS18B20 temperature probes
- reading of tachometer rpm pulses
- display of data using OLED display with rotary data selector
- MUI menu utility for configuring displays and NMEA 2000 data messages
- NMEA 2000 messaging of selected data

Hardware consists of the Hat Labs SH-ESP32 development board. [https://shop.hatlabs.fi/products/sh-esp32](https://docs.hatlabs.fi/sh-esp32/)
and an opensource cloned and built interface board based on the discontinued (https://docs.hatlabs.fi/sh-esp32/add-ons/engine-hat/)
The program is based on the Signal k SensESP project template  (https://github.com/SignalK/SensESP/) 

Comprehensive documentation for SensESP, including how to get started with your own project, is available at the [SensESP documentation site](https://signalk.org/SensESP/).
