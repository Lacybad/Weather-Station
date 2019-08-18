/*
    Weather with display
    Miles Young, 2019
    Modified from HTTP over TLS example, with root CA Certificate
    Original Author: Ivan Grokhotkov, 2017
 */

//Wifi
#include <ESP8266WiFi.h>
#include <WiFiClientSecure.h>
#include <ArduinoJson.h>
#include "src/weatherClass.h"
#include "MiscSettings.h"
//display
#include <TFT_eSPI.h>
#include <SPI.h>
#include "src/BMP_functions.h"
//file system
//use fs::File for SPIFFS, sd::File for SD if needed
#define FS_NO_GLOBALS
#include <FS.h>

/* include a MiscSettings.h file for these defs
    #ifndef STASSID
    #define STASSID "WiFi"
    #define STAPSK  "Password"
    #endif
    #define apikey "xxx"
    #define forecastLoc "/xx.xx,xx.xx"
*/
/* Uncomment in <ArduinoLibrary>/TFT_eSPI/User_Setup.h
    #define ST7735_Driver
    #define ST7735_GREENTAB2
*/

//function defs
void LED(bool led_output);
void setup();
void loop();
void getWeather();
void printIcon();
int checkWeatherIcon(const char *icon);

// Constant variables
const char *ssid = STASSID;
const char *password = STAPSK;
const char *host = "api.darksky.net";
const int httpsPort = 443;
const size_t capacity = JSON_ARRAY_SIZE(8) + JSON_OBJECT_SIZE(3) + JSON_OBJECT_SIZE(6) +
    JSON_OBJECT_SIZE(19) + 4*JSON_OBJECT_SIZE(38) + 4*JSON_OBJECT_SIZE(39) + 6170;

const String forecastType = "/forecast/";
//const String forecastLoc //see define location
const String forecastDetails = "?exclude=minutely,hourly,flags";
const char *weatherIcon[] = {"clear-day", "clear-night", "rain", "snow",
    "sleet", "wind", "fog", "cloudy", "partly-cloudy-day", "partly-cloudy-night",
    "hail", "thunderstorm", "tornado", "na"};
const int weatherIconSize = 14;
//global variables
BearSSL::WiFiClientSecure client;

TFT_eSPI tft = TFT_eSPI(); //start library
CurrentWeather current;

//display vars
uint16_t cursorX;
uint16_t cursorY;

void LED(bool led_output){
    digitalWrite(LED_BUILTIN, !led_output); //need to flip the led
}

void setup() {
    uint8_t i;
    pinMode(LED_BUILTIN, OUTPUT); //LED, GPIO 2, D4
    LED(HIGH);

    Serial.begin(115200);
    tft.init();
    tft.setRotation(2);
    tft.fillScreen(TFT_BLACK);
    tft.setCursor(0,0,2); //start at top, font size 2

    if (!SPIFFS.begin()){
        Serial.println("SPIFFS init failed...");
        while (1) { delay(1); } //stop program
    }
    Serial.println("SPIFFS init");

    WiFi.forceSleepWake(); delay(1);
    Serial.print("\nConnecting to ");
    Serial.println(ssid);
    tft.println("Connecting to:");
    tft.println(ssid);
    WiFi.persistent(false); delay(1);
    WiFi.mode(WIFI_STA); delay(1);
    WiFi.begin(ssid, password);

    i=0;
    cursorY = tft.getCursorY();
    Serial.print("Cursor Y:");
    Serial.println(cursorY);
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
        if (i == 15){
            tft.drawString("                    ", 0, cursorY);
            tft.setCursor(0,cursorY,2);
            i = 0;
        }
        tft.print(".");
        i++;
    }
    Serial.print("Connected, IP address: ");
    Serial.println(WiFi.localIP());
    tft.print("\nIP: ");
    tft.println(WiFi.localIP());

    //cursorY = tft.getCursorY();
    //drawBmp("/na.bmp", 0, cursorY+1);

    LED(LOW);
    /*
    for (i=0; i<weatherIconSize-1; i++){
        printIcon(checkWeatherIcon(weatherIcon[i]));
    }*/
    //printIcon(checkWeatherIcon("dsasdfa"));
    getWeather();
}

void printIcon(int icon){
    tft.fillScreen(TFT_BLACK);
    tft.setCursor(0,0,2);
    tft.println(weatherIcon[icon]);

    char temp[25] = "/";
    strcat(temp, weatherIcon[icon]);
    strcat(temp, ".bmp");

    drawBmp(temp, 0, tft.getCursorY());
    delay(1000);
}

void loop() {
    delay(100);
}

void getWeather() {
    // Connect to remote server
    client.setInsecure(); //no https
    Serial.print("connecting to ");
    Serial.println(host);
    if (!client.connect(host, httpsPort)) {
        Serial.println("connection failed");
        tft.println("connection fail");
        return;
    }

    Serial.print("Getting forecast for: ");
    Serial.println("https://" + String(host) +
            forecastType + apikey + forecastLoc + forecastDetails);

    client.print(String("GET ") + forecastType + apikey +
            forecastLoc + forecastDetails +
            " HTTP/1.1\r\n" +
            "Host: " + host + "\r\n" +
            "User-Agent: ESP8266\r\n" +
            "Connection: close\r\n\r\n");

    Serial.println("request sent");
    while (client.connected()) {
        String line = client.readStringUntil('\n');
        if (line == "\r") {
            Serial.println("headers received");
            tft.println("Got forecast");
            break;
        }
    }

    //help from https://arduinojson.org/v6/assistant/
    Serial.print("Created doc with size:"); Serial.println(capacity);
    DynamicJsonDocument doc(capacity);
    DeserializationError error = deserializeJson(doc, client);

    if (error){
        Serial.print("Parse failed - ");
        Serial.println(error.c_str());
        return;
    }
    current.setupWeather(doc["currently"]);
    Serial.println(current.temperature);

    JsonObject daily = doc["daily"];
    JsonArray daily_data = daily["data"];
    for (int i=0; i<7; i++){
        JsonObject daily_data_0 = daily_data[i];
        float daily_data_0_temperatureHigh = daily_data_0["temperatureHigh"];
        Serial.print(i); Serial.print(" temp ");
        Serial.println(daily_data_0_temperatureHigh);
    }

    Serial.println("Done!");

    WiFi.disconnect(); delay(1);
    WiFi.mode(WIFI_OFF); delay(1);
    WiFi.forceSleepBegin(); delay(1);
}

//can not edit input
int checkWeatherIcon(const char *icon){
    //loop through, string compare
    for (int i=0; i<weatherIconSize; i++){
        if (strcmp(icon, weatherIcon[i]) == 0){
            return i;
        }
    }
    //do not know icon, return default unknown
    return weatherIconSize-1;
}
