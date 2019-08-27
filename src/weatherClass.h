/*
 * weatherClass.h - lib for weather data
 * By Miles Young, Aug 2019
 */

#ifndef weatherClass_h
#define weatherClass_h

#include <Arduino.h>
#include <ArduinoJson.h>

class Weather {
    private:
        bool setup;
        bool daily;
        long time;
        const char *icon;
        long sunriseTime;
        long sunsetTime;
        int precipProbability;
        float temperatureLong;
        int temperature;
        float temperatureHighLong;
        int temperatureHigh;
        float temperatureLowLong;
        int temperatureLow;
        int humidity;

    public:
        Weather();
        bool setupWeather(JsonObject weatherData);
        bool getSetup();
        bool isDaily();
        long getTime();
        const char* getIcon();
        long getSunriseTime();
        long getSunsetTime();
        int getPrecipProb();
        int getTemp();
        int getTempHigh();
        int getTempLow();
        int getHumidity();
};

#endif
