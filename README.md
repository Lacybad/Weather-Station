# Weather Station

This weather station uses an ESP8266, a TFT KMR-1.8 LCD display, a PIR motion sensor, BJTs
and a photoresistor for the display brightness, and an option 1-wire temperature sensor
(no code for it right now).

This weather station uses darksky for the forecast with a free account. It displays the
current weather temperature, a high and low, and the next two days.

In Settings.h (or MiscSettings.h), add the GPS coordinates, wifi information, and timezone
to make the system work.

This project's PCB was made using JLCPCB. Using the cheapest shipping, it took 6 days to
LAX, a week in customs, and then 3-5 days to a location in the US.

## Schematic

![Schematic](weatherPCB/WeatherPCB.svg)