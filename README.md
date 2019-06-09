# STM32_RTTY
STM32 &amp; SI4032 rtty test

Released under GPL v2

Use:
https://www.wyzbee.com/download/Utilities/Software/CoIDE-1.7.8.exe

And:
https://launchpad.net/gcc-arm-embedded/5.0/5-2016-q3-update/+download/gcc-arm-none-eabi-5_4-2016q3-20160926-win32.exe

Using Linux:
cd into main folder
cmake .
make

Have a nice day ;)

#Changelog
 * 14.12.2016 - Reverse engineeded connections, initial hard work, resulting in working RTTY by SQ7FJB
 * 07.01.2017 - GPS now using proprietiary UBLOX protocol, more elastic code to set working frequency by SQ5RWU
 * 23.01.2017 - Test APRS code, small fixes in GPS code by SQ5RWU
 * 06.06.2017 - APRS code fix, some code cleanup


#TODO
 * More APRS config options
 * Temperature and moisture sensor?

# Configuration
All configs in ```config.h```
Shift 450Hz

* ```CALLSIGN``` RTTY callsign
* ```APRS_CALLSIGN``` APRS callsign, 6 characters. If your callsign is shorter add spaces
* ```APRS_COMMENT``` APRS comment
* ```APRS_SSID``` APRS SSID - '1' -> 1, 'A' -> 10 etc.
* ```RTTY_TO_APRS_RATIO``` number of RTTY frames between each APRS frame
* ```RTTY_FREQUENCY``` RTTY frequency in MHz
* ```APRS_FREQUENCY``` APRS frequency in MHz
* ```RTTY_SPEED``` RTTY speed in bauds
* ```RTTY_7BIT``` Use 7 bit RTTY
* ```RTTY_USE_2_STOP_BITS``` use 2 stop bits
* ```TX_POWER``` Power 0-7, where 7 means 42.95 mW@434.150 MHz measured on E4406A
* ```tx_delay``` Delay between frames in milliseconds
* ```ALLOW_DISABLE_BY_BUTTON``` Allow disabling device using button