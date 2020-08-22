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
        uint8_t iconNum;
        long sunriseTime;
        long sunsetTime;
        int precipProbability;
        float precipAmount;
        float temperatureLong;
        int temperature;
        float temperatureHighLong;
        int temperatureHigh;
        float temperatureLowLong;
        int temperatureLow;
        int humidity;

    public:
        Weather();
        bool setupWeather(JsonObject weatherData, bool ifDaily);
        bool getSetup();
        bool isDaily();
        long getTime();
        const char* getIcon();
        void setIconNum(uint8_t num); //for reference
        uint8_t getIconNum();
        long getSunriseTime();
        long getSunsetTime();
        int getPrecipProb();
        float getPrecipAmt();
        int getTemp();
        int getTempHigh();
        int getTempLow();
        int getHumidity();
};

#endif
