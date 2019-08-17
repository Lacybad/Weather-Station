SHELL := /bin/bash
ARDUINO_IDE = $(HOME)/Documents/ArduinoIDE
ARDUINO_PRGM = $(ARDUINO_IDE)/arduino

ARDUINO_INO_FILE = Weather
SRC = $(ARDUINO_INO_FILE).ino
DATA = data
WI = weather-icons/svg
WIFLAGS = -density 200 -resize 48x48 -channel RGB -negate  -type truecolor

default: upload

gui: openGUI
openGUI: 
	$(ARDUINO_PRGM) $(SRC)

upload:
	$(ARDUINO_PRGM) $(VBS) --upload $(SRC)

compile:
	$(ARDUINO_PRGM) $(VBS) --verify $(SRC)

verbose-upload: VBS = -v
verbose-upload: upload

verbose-compile: VBS = -v
verbose-compile: compile

#convert all in bash
#using icons from https://github.com/erikflowers/weather-icons
imageConvert:
	mkdir -p $(DATA) 	
	cp $(WI)/wi-day-sunny.svg $(DATA)/clear-day.svg
	cp $(WI)/wi-night-clear.svg $(DATA)/clear-night.svg
	cp $(WI)/wi-rain.svg $(DATA)/rain.svg
	cp $(WI)/wi-snow.svg $(DATA)/snow.svg
	cp $(WI)/wi-sleet.svg $(DATA)/sleet.svg
	cp $(WI)/wi-strong-wind.svg $(DATA)/wind.svg
	cp $(WI)/wi-fog.svg $(DATA)/fog.svg
	cp $(WI)/wi-cloudy.svg $(DATA)/cloudy.svg
	cp $(WI)/wi-day-cloudy.svg $(DATA)/partly-cloudy-day.svg
	cp $(WI)/wi-night-cloudy.svg $(DATA)/partly-cloudy-night.svg
	cp $(WI)/wi-hail.svg $(DATA)/hail.svg
	cp $(WI)/wi-thunderstorm.svg $(DATA)/thunderstorm.svg
	cp $(WI)/wi-tornado.svg $(DATA)/tornado.svg
	cp $(WI)/wi-na.svg $(DATA)/na.svg
	for i in $(DATA)/*.svg; do \
		echo -e "Converting $$i"; \
		convert $(WIFLAGS) $$i $${i%.svg}.bmp; \
	done
	-rm -f $(DATA)/*.svg 
