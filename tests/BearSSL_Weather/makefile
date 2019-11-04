ARDUINO_IDE = $(HOME)/Documents/ArduinoIDE
ARDUINO_PRGM = $(ARDUINO_IDE)/arduino

ARDUINO_INO_FILE = BearSSL_Validation
ifneq ($(wildcard *.ino).ino,)
ARDUINO_INO_FILE = $(wildcard *.ino)
else
ARDUINO_INO_FILE = Weather.ino
endif
SRC = $(ARDUINO_INO_FILE)

default: upload

gui:
	$(ARDUINO_PRGM) $(SRC)

upload:
	$(ARDUINO_PRGM) $(VBS) --upload $(SRC)

compile:
	$(ARDUINO_PRGM) $(VBS) --verify $(SRC)

verbose-upload: VBS = -v
verbose-upload: upload

verbose-compile: VBS = -v
verbose-compile: compile

