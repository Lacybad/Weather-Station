/*
 * weatherClass.cpp - lib for weather data
 * By Miles Young, Aug 2019
 */

#include "weatherClass.h"
#include <Arduino.h>
#include <ArduinoJson.h>

CurrentWeather::CurrentWeather() {
    setup = false;
}

void CurrentWeather::setupWeather(JsonObject current) {
    time = current["time"];
    icon = current["icon"];
    temperature = current["apparentTemperature"];
}
