/*
 * weatherClass.h - lib for weather data
 * By Miles Young, Aug 2019
 */

#ifndef weatherClass_h
#define weatherClass_h

#include <Arduino.h>
#include <ArduinoJson.h>

class CurrentWeather {
    private:
        bool setup;
        long time;
        const char *icon;
        float temperature;

    public:
        CurrentWeather();
        void setupWeather(JsonObject current);
        bool getSetup();
        long getTime();
        const char* getIcon();
        float getTemp();
};

class DailyWeather {
    private:
        bool setup;
        long time;
        const char *icon;
        long sunriseTime;
        long sunsetTime;
        float precipProbability;
        float temperatureHigh;
        float temperatureLow;
        float humidity;

    public:
        DailyWeather();
        void setupWeather(JsonObject daily);
        bool getSetup();
};

#endif
