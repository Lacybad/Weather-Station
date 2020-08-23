# Weather Station

<img align="right" width="300"
src="https://user-images.githubusercontent.com/10273995/69016963-2e7df480-0958-11ea-8470-089bf50c75a8.jpg">
This weather station uses an ESP8266 (WeMos D1 Mini), a TFT KMR-1.8 LCD display, a PIR
motion sensor, BJTs and a photoresistor for the display brightness, and an optional 1-wire
temperature sensor (no code for it right now).

This weather station uses darksky for the forecast with a free account. It displays the
current weather temperature, a high and low, and the next two days.

In Settings.h (or MiscSettings.h), add the GPS coordinates, wifi information, and timezone
to make the system work.

This project's PCB was made using JLCPCB. Using the cheapest shipping, it took 6 days to
LAX, a week in customs, and then 3-5 days to a location in the US.

## Icons / Other Code

The icons used for this project are from
[erikflowers](https://github.com/erikflowers/weather-icons).  Using the makefile command
`make imageConvert` converts the images in the folder weather-icons/svg to bmps in a data
folder. This folder can then be uploaded to the ESP8266 using the
[data-uploader](https://github.com/esp8266/arduino-esp8266fs-plugin), which uploads the
data in the /data folder to the SPIFFS file system. It is recommended if using the D1 mini
to have at least 1 MB of storage, and the images take up 124.0 KiB of space.

Other code for this project is from Bodmer's TFT\_eSPI library example for the
[BMP\_functions.ino](https://github.com/Bodmer/TFT_eSPI/blob/master/examples/Generic/TFT_SPIFFS_BMP/BMP_functions.ino),
and converted to work as a C++ library. The modified
[cpp](https://github.com/mwyoung/Weather-Station/blob/master/src/BMP_functions.cpp),
[h](https://github.com/mwyoung/Weather-Station/blob/master/src/BMP_functions.h) and
[license](https://github.com/mwyoung/Weather-Station/blob/master/src/BMP_functions_license.txt)
are in the src directory.

All other libraries (with given versions used) are not really modified from the source
(besides the [TFT\_eSPI v1.4.20](https://github.com/Bodmer/TFT_eSPI) for the pins and
display), and include [ESP8266 Board v2.5.2](https://github.com/esp8266/Arduino) with docs
[ESP8266Wifi and WiFiClientSecure](https://arduino-esp8266.readthedocs.io/en/latest/esp8266wifi/readme.html),
[ArduinoJson v6.13.0](https://github.com/bblanchon/ArduinoJson),
[BearSSL v1.4.0](https://github.com/arduino-libraries/ArduinoBearSSL),
[SPI - ESP8266](https://github.com/esp8266/Arduino/blob/master/libraries/SPI/SPI.h),
[Ticker - ESP8266](https://github.com/esp8266/Arduino/blob/master/libraries/Ticker/src/Ticker.h),
[Timezone v1.2.2](https://github.com/JChristensen/Timezone) and
[TimeLib v1.5.0](https://github.com/PaulStoffregen/Time).

## Data Source
This project originally used DarkSky as a source but moved to
[OpenWeatherMap](https://openweathermap.org/darksky-openweather) after DarkSky shut down
their API. The DarkSky branch in this repo contains the original code that was working
with that API.

## Schematic

<img align="center" src="weatherPCB/WeatherPCB.svg" />


For the PCB, the [d1\_mini\_kicad](https://github.com/jerome-labidurie/d1_mini_kicad)
library was used along with the
[digikey-kicad-library](https://github.com/Digi-Key/digikey-kicad-library) and a few
default libraries KiCad for the transistors. The parts can be found in the
[DigiKey purchase list](https://github.com/mwyoung/Weather-Station/blob/master/weatherPCB/DigikeyCartExample.csv),
and this csv file can be uploaded to digikey to buy the parts. Extra parts are included as
backup for buying parts along with buying a large quantity means cost savings. It is also
possible to use a check to save on shipping (could take an extra few days)

Other parts are from Amazon but can be bought on other sites or vendors if it is cheaper.
The parts include a [Wemos d1 mini](https://www.amazon.com/gp/product/B076F53B6S/),
[Photoresistors](https://www.amazon.com/gp/product/B01N0GJ6QP/),
[IR PIR Motion Sensor](https://www.amazon.com/gp/product/B07LF47TRC/),
[LCD Display](https://www.amazon.com/gp/product/B00LSG51MM/),
[Project Boxes](https://www.amazon.com/dp/B07V7X11LJ).
