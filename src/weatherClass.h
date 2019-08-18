/*
 * weatherClass.h - lib for weather data
 * By Miles Young, Aug 2019
 */

#ifndef weatherClass_h
#define weatherClass_h

#include <Arduino.h>
#include <ArduinoJson.h>

class CurrentWeather {
    public:
        CurrentWeather();
        void setupWeather(JsonObject current);
        bool setup = false;
        long time;
        const char *icon;
        float temperature;
};

//class dailyWeather {

//}

#endif
