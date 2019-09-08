SHELL := /bin/bash
ARDUINO_IDE := $(HOME)/Documents/ArduinoIDE
ARDUINO_PRGM := $(ARDUINO_IDE)/arduino
ARDUINO_IDE_LIBS := $(ARDUINO_IDE)/libraries
ARDUINO_WORKSPACE := $(HOME)/Documents/Arduino
ARDUINO_LIBS := $(ARDUINO_WORKSPACE)/libraries
ARDUINO_OLDLIBS := $(ARDUINO_WORKSPACE)/OldLibraries
ESP_LIBS := $(HOME)/.arduino15/packages/esp8266/hardware/esp8266/2.5.2/libraries

ARD_TAG = tags.arduino_lib
ESP_TAG = tags.esp8266_lib
PRJ_TAG = tags.ino
TAG_FLAGS = --langmap=c++:.cpp.ino.pde.h.hpp --languages=C++

ifneq ($(wildcard *.ino).ino,)
ARDUINO_INO_FILE = $(wildcard *.ino)
else
ARDUINO_INO_FILE = Weather.ino
endif
SRC = $(ARDUINO_INO_FILE)
DATA = data

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

#Help from: https://collectiveidea.com/blog/archives/2017/04/05/arduino-programming-in-vim
ctags:
	ctags -f $(ARD_TAG) --tag-relative=never --exclude=$(ARDUINO_OLDLIBS)/* $(TAG_FLAGS) -R $(ARDUINO_LIBS)
	ctags -f $(ESP_TAG) --tag-relative=never $(TAG_FLAGS) -R $(ESP_LIBS)
	ctags -f $(PRJ_TAG) $(TAG_FLAGS) -R .
	cat $(ARD_TAG) $(ESP_TAG) $(PRJ_TAG) > tags
	sort tags -o tags
	rm -f tags.*
