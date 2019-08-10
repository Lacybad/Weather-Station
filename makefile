ARDUINO_IDE = $(HOME)/Documents/ArduinoIDE
ARDUINO_PRGM = $(ARDUINO_IDE)/arduino

ARDUINO_INO_FILE = Weather
SRC = $(ARDUINO_INO_FILE).ino

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

