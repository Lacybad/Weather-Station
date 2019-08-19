SHELL := /bin/bash
ARDUINO_IDE = $(HOME)/Documents/ArduinoIDE
ARDUINO_PRGM = $(ARDUINO_IDE)/arduino

ARDUINO_INO_FILE = Weather
SRC = $(ARDUINO_INO_FILE).ino
DATA = data

#Weather Images
WI = weather-icons/svg
#Flags - make density large, then resize, flip colors, set 24 bit color
WIFLAG_COLOR = -channel RGB -negate  -type truecolor
WIFLAG_LARGE = -density 200 -resize 48x48 $(WIFLAG_COLOR)
WIFLAG_SMALL = -density 200 -resize 24x24 $(WIFLAG_COLOR)

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
imageConvertLarge:
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
		convert $(WIFLAG_LARGE) $$i $${i%.svg}.bmp; \
	done
	-rm -f $(DATA)/*.svg

imageConvertSmall:
	mkdir -p $(DATA)
	cp $(WI)/wi-sunrise.svg $(DATA)/sunrise.svg
	cp $(WI)/wi-sunset.svg $(DATA)/sunset.svg
	for i in $(DATA)/*.svg; do \
		echo -e "Converting $$i"; \
		convert $(WIFLAG_SMALL) $$i $${i%.svg}.bmp; \
	done
	-rm -f $(DATA)/*.svg

imageConvert: imageConvertLarge	imageConvertSmall

