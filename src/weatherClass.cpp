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
    //https://arduinojson.org/v6/assistant/
    const char* error = weatherData["clouds"];
    if (error) { //check if null or error
        setup = false;
        return false;
    }
    daily = ifDaily;

    time = weatherData["dt"]; //time of forecast

    const char* temp = weatherData["weather"][0]["icon"];

    //loop through possible combinations
    const char* owmIcons[] = {"01d", "01n", "02d", "02n",
        "03d", "03n", "04d", "04n", "09d", "09n", "10d", "10n",
        "11d", "11n", "13d", "13n", "50d", "50n"};
    const uint8_t owmSize = 9*2;
    //possible names from files
    const char* dkIcons[] = {"clear-day", "clear-night", "partly-cloudy-day", "partly-cloudy-night",
        "cloudy", "cloudy", "cloudy", "cloudy", "rain", "rain", "rain", "rain",
        "tunderstorm", "thunderstorm", "snow", "snow", "fog", "fog"};
    for (int i=0; i<owmSize; i++){
        if (strcmp(owmIcons[i], temp) == 0){
            icon = dkIcons[i];
        }
    }
    if (icon == NULL){
        icon = "na";
    }

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
        precipAmount = tempPrecip/24.5f; //mm to inches
    }

    const String tempStr = "temp"; //temp or feels_like
    //temperature
    if(daily){
        temperatureHighLong = weatherData["temp"]["day"];
        temperatureLowLong = weatherData["temp"]["night"];
        temperatureLong = temperatureHighLong;
        temperatureApproxLong = weatherData["feels_like"]["day"];
    }
    else {
        temperatureLong = weatherData["temp"];
        temperatureHighLong = temperatureLong;
        temperatureLowLong = temperatureLong;
        temperatureApproxLong = weatherData["feels_like"];
    }
    //get integers
    temperature = round(temperatureLong);
    temperatureHigh = round(temperatureHighLong);
    temperatureLow = round(temperatureLowLong);
    temperatureApprox = round(temperatureApproxLong);

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

int Weather::getTempApprox(){
    return temperatureApprox;
}

int Weather::getHumidity(){
    return humidity;
}
