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
#include <ezTime.h>

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
void printWeatherDisplay();
bool printWeatherSerial();
void printIcon(const char *icon);
int checkWeatherIcon(const char *icon);
void clearScreen(int textSize);
void setTextSize(int textSize);
void connectToWifi();
void disconnectWifi();

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

//weather variables
Weather currentWeather;
Weather dailyWeather[3]; //weather for today, tomorrow, and day+1
const int dailyWeatherSize = 3;

//display vars
uint16_t cursorX;
uint16_t cursorY;

void LED(bool led_output){
    digitalWrite(LED_BUILTIN, !led_output); //need to flip the led
}

void setup() {
    pinMode(LED_BUILTIN, OUTPUT); //LED, GPIO 2, D4
    LED(HIGH);

    Serial.begin(115200);
    tft.init();
    tft.setRotation(2);
    clearScreen(2); //start at top, font size 2

    if (!SPIFFS.begin()){
        Serial.println("SPIFFS init failed...");
        while (1) { delay(1); } //stop program
    }
    Serial.println("SPIFFS init");

    connectToWifi();
    LED(LOW);

    //cursorY = tft.getCursorY();
    //drawBmp("/na.bmp", 0, cursorY+1);

    getWeather();
    printWeatherSerial();
    printWeatherDisplay();
}

void loop() {
    delay(100);
}

void getWeather() {
    clearScreen(2);

    // Connect to remote server
    client.setInsecure(); //no https
    Serial.print("connecting to ");
    Serial.println(host);
    if (!client.connect(host, httpsPort)) {
        Serial.println("connection failed");
        tft.println("connection fail");
        client.stop();
        return;
    }
    else {
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
    }

    //help from https://arduinojson.org/v6/assistant/
    Serial.print("Created doc with size:"); Serial.println(capacity);
    DynamicJsonDocument doc(capacity);
    DeserializationError error = deserializeJson(doc, client);

    if (error){
        Serial.print("Parse failed - ");
        Serial.println(error.c_str());
        tft.println("JSON parse failed");
        return;
    }
    bool output = currentWeather.setupWeather(doc["currently"]);
    if (output == false){
        Serial.println("Setup failed for current");
        return;
    }
    for (int i=0; i<dailyWeatherSize; i++){
        output = dailyWeather[i].setupWeather(doc["daily"]["data"][i]);
        if (output == false){
            Serial.println("Setup failed for daily");
            return;
        }
    }

    Serial.println("Done!");
}

void printWeatherDisplay(){
    clearScreen(2);

    tft.print("Currently: ");
    tft.println(currentWeather.getTimeLong());
    printIcon(currentWeather.getIcon());
    tft.setCursor(tft.getCursorX()+48,tft.getCursorY(), 4);
    tft.println(currentWeather.getTemp());
    setTextSize(2);
    tft.println("\nhere");

    setDebug(INFO);
    waitForSync();
    Timezone myTz;
    myTz.setLocation("America/Los_Angeles");
    Serial.println(myTz.dateTime("d-M-y g:i A T"));

    time_t currentTime = (time_t)currentWeather.getTimeLong();
    Serial.println(currentTime);
    Serial.println(myTz.dateTime(currentTime, "d-M-y g:i A T"));

    currentTime = (time_t)dailyWeather[0].getTimeLong();
    Serial.println(currentTime);
    Serial.println(myTz.dateTime(currentTime, "d-M-y g:i A T"));
}

bool printWeatherSerial(){
    if (currentWeather.getSetup()){
        Serial.println("=================");
        Serial.println(currentWeather.getTemp());
        Serial.println(currentWeather.getSunriseTime());
        Serial.println(currentWeather.getTempHigh());
        Serial.println(currentWeather.getHumidity());
    }
    else {
        return false;
    }
    Serial.println("=================");
    for (int i=0; i<dailyWeatherSize; i++){
        if (dailyWeather[i].getSetup()){
            Serial.println(dailyWeather[i].getTemp());
            Serial.println(dailyWeather[i].getSunriseTime());
            Serial.println(dailyWeather[i].getTempHigh());
            Serial.println(dailyWeather[i].getHumidity());
            Serial.println("=================");
        }
        else {
            return false;
        }
    }
}

void printIcon(const char *icon){
    int iconNum = checkWeatherIcon(icon);

    char temp[25] = "/";
    strcat(temp, weatherIcon[iconNum]);
    strcat(temp, ".bmp");

    drawBmp(temp, 0, tft.getCursorY());
    delay(1000);
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

void clearScreen(int textSize){
    tft.fillScreen(TFT_BLACK);
    tft.setCursor(0,0,textSize);
}

void setTextSize(int textSize){
    cursorX = tft.getCursorX();
    cursorY = tft.getCursorY();
    if (cursorX != 0){
        cursorY++;
        cursorX = 0;
    }
    tft.setCursor(cursorX,cursorY,textSize);
}

void connectToWifi(){
    WiFi.forceSleepWake(); delay(1);
    Serial.print("\nConnecting to ");
    Serial.println(ssid);
    tft.println("Connecting to:");
    tft.println(ssid);
    WiFi.persistent(false); delay(1);
    WiFi.mode(WIFI_STA); delay(1);
    WiFi.begin(ssid, password);

    int i = 0;
    cursorY = tft.getCursorY();
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
}

void disconnectWifi(){
    WiFi.disconnect();
    delay(1);
    WiFi.mode(WIFI_OFF);
    delay(1);
    WiFi.forceSleepBegin();
    delay(1);
}
