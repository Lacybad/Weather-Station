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

bool Weather::setupWeather(JsonObject weatherData, bool ifDaily) {
    //const char* weatherType;
    //https://arduinojson.org/v6/api/jsonobject/containskey/
    const char* error = weatherData["clouds"];
    if (error) { //check if null or error
        setup = false;
        return false;
    }
    daily = ifDaily;

    time = weatherData["dt"]; //time of forecast

    const char* tempIcon = weatherData["weather"]["icon"];

    iconNum = 0; //by default

    //sunset/sunrise time
    if (daily){
        sunriseTime = weatherData["sunrise"];
        sunsetTime = weatherData["sunset"];
    }
    else {
        sunriseTime = 0;
        sunsetTime = 0;
    }

    float tempPrecip = weatherData["pop"]; //precipitation probability
    precipProbability = (int)(100*tempPrecip); //convert to int
    if (daily){
        tempPrecip = weatherData["rain"];
        //precipAmount = (((int)(100*tempPrecip)*24) / 100); //over 24 hours, round
        //precipitation amount over 24 hours
        precipAmount = ((tempPrecip*24)*100)/100;
    }

    const String tempStr = "temp"; //temp or feels_like
    //temperature
    if(daily){
        temperatureHighLong = weatherData[tempStr]["day"];
        temperatureLowLong = weatherData[tempStr]["night"];
        temperatureLong = temperatureHighLong;
    }
    else {
        temperatureLong = weatherData[tempStr];
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

float Weather::getPrecipAmt(){
    return precipAmount;
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
