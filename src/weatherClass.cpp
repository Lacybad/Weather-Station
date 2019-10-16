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
    iconNum = 0; //by default

    //sunset/sunrise time
    if (daily){
        sunriseTime = weatherData["sunriseTime"];
        sunsetTime = weatherData["sunsetTime"];
    }
    else {
        sunriseTime = 0;
        sunsetTime = 0;
    }

    float tempPrecip = weatherData["precipProbability"];
    precipProbability = (int)(100*tempPrecip);

    //temperature
    if(daily){
        temperatureHighLong = weatherData["apparentTemperatureHigh"];
        temperatureLowLong = weatherData["apparentTemperatureLow"];
        temperatureLong = temperatureHighLong;
    }
    else {
        temperatureLong = weatherData["apparentTemperature"];
        temperatureHighLong = temperatureLong;
        temperatureLowLong = temperatureLong;
    }
    temperature = round(temperatureLong);
    temperatureHigh = round(temperatureHighLong);
    temperatureLow = round(temperatureLowLong);

    float humidityLong = weatherData["humidity"];
    humidity = (int)round(100*humidityLong);

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

void Weather::setIconNum(uint8_t num){
    iconNum = num; //optional
}

uint8_t Weather::getIconNum(){
    return iconNum;
}

long Weather::getSunriseTime(){
    return sunriseTime;
}

long Weather::getSunsetTime(){
    return sunsetTime;
}

int Weather::getPrecipProb(){
    return precipProbability;
}

int Weather::getTemp(){
    return temperature;
}

int Weather::getTempHigh(){
    return temperatureHigh;
}

int Weather::getTempLow(){
    return temperatureLow;
}

int Weather::getHumidity(){
    return humidity;
}
