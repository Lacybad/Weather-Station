/*
 * weatherClass.cpp - lib for weather data
 * By Miles Young, Aug 2019
 */

#include "weatherClass.h"
#include <Arduino.h>
#include <ArduinoJson.h>

// weather - both current and daily
Weather::Weather() {
    setup = false;
}

bool Weather::setupWeather(JsonObject weatherData) {
    //const char* weatherType;
    //https://arduinojson.org/v6/api/jsonobject/containskey/
    //weatherType = weatherData["temperatureHigh"]; //check if daily
    //if (weatherType) { //check if not null
    if (weatherData.containsKey("temperatureHigh")) { //check if not null
        daily = true;
    }
    else {
        //weatherType = weatherData["temperature"]; //check if current
        if (weatherData.containsKey("temperature")){
            daily = false;
        }
        else { //is an error
            setup = false;
            return false;
        }
    }

    time = weatherData["time"];
    icon = weatherData["icon"];

    //sunset/sunrise time
    if (daily){
        sunriseTime = weatherData["sunriseTime"];
        sunsetTime = weatherData["sunsetTime"];
    }
    else {
        sunriseTime = 0;
        sunsetTime = 0;
    }

    precipProbability = weatherData["precipProbability"];

    //temperature
    if(daily){
        temperatureHigh = weatherData["apparentTemperatureHigh"];
        temperatureLow = weatherData["apparentTemperatureLow"];
        temperature = temperatureHigh;
    }
    else {
        temperature = weatherData["apparentTemperature"];
        temperatureHigh = temperature;
        temperatureLow = temperature;
    }

    humidity = weatherData["humidity"];

    setup = true;
    return true;
}

//getter functions
bool Weather::getSetup(){
    return setup;
}

long Weather::getTime(){
    return time;
}

const char* Weather::getIcon(){
    return icon;
}

long Weather::getSunriseTime(){
    return sunriseTime;
}

long Weather::getSunsetTime(){
    return sunsetTime;
}

float Weather::getTemp(){
    return temperature;
}

float Weather::getTempHigh(){
    return temperatureHigh;
}

float Weather::getTempLow(){
    return temperatureLow;
}

float Weather::getHumidity(){
    return humidity;
}
