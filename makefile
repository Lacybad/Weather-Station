SHELL := /bin/bash
ARDUINO_IDE = $(HOME)/Documents/ArduinoIDE
ARDUINO_PRGM = $(ARDUINO_IDE)/arduino

ARDUINO_INO_FILE = Weather
SRC = $(ARDUINO_INO_FILE).ino
DATA = data
DATA_PNG = data_png

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
convertImages:
	if [ ! -f $(DATA)/*.png ]; then \
		echo "no png files"; \
		exit 1; \
	fi
	mkdir -p $(DATA) $(DATA_PNG)	
	for i in $(DATA)/*.png; do \
		convert $$i -background black -alpha remove -flatten -alpha off $${i%.png}.bmp; \
	done
	mv $(DATA)/*.png $(DATA_PNG)
