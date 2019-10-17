# Weather Station

This weather station uses an ESP8266 (WeMos D1 Mini), a TFT KMR-1.8 LCD display, a PIR
motion sensor, BJTs and a photoresistor for the display brightness, and an option 1-wire
temperature sensor (no code for it right now).

This weather station uses darksky for the forecast with a free account. It displays the
current weather temperature, a high and low, and the next two days.

In Settings.h (or MiscSettings.h), add the GPS coordinates, wifi information, and timezone
to make the system work.

This project's PCB was made using JLCPCB. Using the cheapest shipping, it took 6 days to
LAX, a week in customs, and then 3-5 days to a location in the US.

## Icons

The icons used for this project are from
[erikflowers](https://github.com/erikflowers/weather-icons).  Using the makefile command
`make imageConvert` converts the images in the folder weather-icons/svg to bmps in a data
folder. This folder can then be uploaded to the ESP8266 using the
[data-uploader](https://github.com/esp8266/arduino-esp8266fs-plugin), which uploads the
data in the /data folder to the SPIFFS file system. It is recommended if using the D1 mini
to have at least 1 MB of storage, and the images take up 124.0 KiB of space.

## Schematic

<img align="center" src="weatherPCB/WeatherPCB.svg" />
