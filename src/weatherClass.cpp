/*
 * weatherClass.cpp - lib for weather data
 * By Miles Young, Aug 2019
 */

#include "weatherClass.h"
#include <Arduino.h>
#include <ArduinoJson.h>

//current weather
CurrentWeather::CurrentWeather() {
    setup = false;
}

void CurrentWeather::setupWeather(JsonObject current) {
    time = current["time"];
    icon = current["icon"];
    temperature = current["apparentTemperature"];
}

//getter functions
bool CurrentWeather::getSetup(){
    return setup;
}

long CurrentWeather::getTime(){
    return time;
}

const char* CurrentWeather::getIcon(){
    return icon;
}

float CurrentWeather::getTemp(){
    return temperature;
}

/* ============================================
                 Daily Weather
   ============================================ */
DailyWeather::DailyWeather() {
    setup = false;
}

void DailyWeather::setupWeather(JsonObject daily) {
    setup = false;
}

//getter functions
bool DailyWeather::getSetup(){
    return setup;
}
