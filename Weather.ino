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
#include <TFT_eSPI.h> //D4=RST,D8=CS,D3=A0/DC,D7=SDA,D5=SCL
#include <SPI.h>
#include "src/BMP_functions.h"
#define LARGE_ICON 48
#define SMALL_ICON 24
#define TIME_ICON 8     //size of time XX:XX XM
//file system
//use fs::File for SPIFFS, sd::File for SD if needed
#define FS_NO_GLOBALS
#include <FS.h>
#include <Timezone.h>
#include <TimeLib.h>

/* include a MiscSettings.h file for these defs
    #ifndef STASSID
    #define STASSID "WiFi"
    #define STAPSK  "Password"
    #endif
    #define apikey "xxx"
    #define forecastLoc "/xx.xx,xx.xx"
    #define daylightRuleConfig {"*DT", Second, Sun, Mar, 2, -XX0}
    #define standardRuleConfig {"*ST", First, Sun, Nov, 2, -XX0}
*/
/* Uncomment in <ArduinoLibrary>/TFT_eSPI/User_Setup.h
    #define ST7735_Driver
    #define ST7735_GREENTAB2
*/

// Constant variables
const char *ssid = STASSID;
const char *password = STAPSK;
const char *host = "api.darksky.net";
const int httpsPort = 443;
const size_t capacity = JSON_ARRAY_SIZE(8) + JSON_OBJECT_SIZE(3) + JSON_OBJECT_SIZE(6) +
    JSON_OBJECT_SIZE(19) + 4*JSON_OBJECT_SIZE(38) + 4*JSON_OBJECT_SIZE(39) + 6180;

const String forecastType = "/forecast/";
//const String forecastLoc //see define location
const String forecastDetails = "?exclude=minutely,hourly,flags,alerts";
#define SUNRISE_ICON "sunrise"
#define SUNSET_ICON "sunset"
const char *weatherIcon[] = {"clear-day", "clear-night", "rain", "snow",
    "sleet", "wind", "fog", "cloudy", "partly-cloudy-day", "partly-cloudy-night",
    "hail", "thunderstorm", "tornado", SUNRISE_ICON, SUNSET_ICON, "na"};
const int weatherIconSize = 16;

//global variables
BearSSL::WiFiClientSecure client;
TFT_eSPI tft = TFT_eSPI(); //start library

//weather variables
Weather currentWeather;
Weather dailyWeather[3]; //weather for today, tomorrow, and day+1
const int dailyWeatherSize = 3;
TimeChangeRule daylightRule = daylightRuleConfig;
TimeChangeRule standardRule = standardRuleConfig;
Timezone tz(daylightRule, standardRule);

//display vars
uint16_t cursorX;
uint16_t cursorY;
char displayOutput[10];

//function defs
void LED(bool led_output);
void setup();
void loop();
bool getWeather();
void printWeatherDisplay();
void timeToLocal(time_t currentTime);
bool printWeatherSerial();
void printIcon(const char *icon);
int checkWeatherIcon(const char *icon);
void clearScreen(int textSize);
void setTextSize(int textSize);
void connectToWifi();
void disconnectWifi();

void LED(bool led_output){
    digitalWrite(LED_BUILTIN, !led_output); //need to flip the led
}

void setup() {
    pinMode(LED_BUILTIN, OUTPUT); //LED, GPIO 2, D4
    LED(HIGH);

    Serial.begin(115200);
    Serial.println();
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

    bool output = getWeather();
    if (output == true){
        printWeatherSerial();
        printWeatherDisplay();
    }
    else {
        Serial.println("Parse FAILED");
        tft.println("Parse FAILED");
    }
}

void loop() {
    delay(100);
}

bool getWeather() {
    clearScreen(2);

    // Connect to remote server
    client.setInsecure(); //no https
    Serial.print("connecting to ");
    Serial.println(host);
    if (!client.connect(host, httpsPort)) {
        Serial.println("connection failed");
        tft.println("connection fail");
        client.stop();
        return false;
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
        return false;
    }
    bool output = currentWeather.setupWeather(doc["currently"]);
    if (output == false){
        Serial.println("Setup failed for current");
        return false;
    }
    for (int i=0; i<dailyWeatherSize; i++){
        output = dailyWeather[i].setupWeather(doc["daily"]["data"][i]);
        if (output == false){
            Serial.println("Setup failed for daily");
            return false;
        }
    }

    Serial.println("Done!");
    return true;
}

void printWeatherDisplay(){
    clearScreen(1);
    uint16_t iconY;
    uint16_t iconX;

    //current weather
    tft.print("Updated: ");
    timeToLocal(currentWeather.getTime());
    tft.println(displayOutput);

    iconY = tft.getCursorY();
    printIcon(currentWeather.getIcon());
    tft.setCursor(tft.getCursorX()+LARGE_ICON+2,tft.getCursorY()+2, 4);
    tft.print(currentWeather.getTemp());
    tft.drawCircle(tft.getCursorX()+4,tft.getCursorY()+2, 2, TFT_WHITE); //degree symbol
    tft.setCursor(tft.getCursorX()+10,tft.getCursorY(), 2);
    tft.println("F");

    tft.setCursor(tft.getCursorX()+LARGE_ICON+4,tft.getCursorY()+4,2);
    tft.print(dailyWeather[0].getTempHigh());
    tft.print(" / ");
    tft.println(dailyWeather[0].getTempLow());
    tft.setCursor(tft.getCursorX()+LARGE_ICON+4,tft.getCursorY() - 2, 2);
    tft.print("Rain: ");
    tft.print(currentWeather.getPrecipProb());
    tft.println("%");

    cursorY = tft.getCursorY();
    tft.setCursor(2,cursorY,1);
    timeToLocal(dailyWeather[0].getSunriseTime());
    tft.println(displayOutput);
    tft.setCursor(64-(SMALL_ICON >> 1),cursorY); //divide 2
    if (currentWeather.getTime() < dailyWeather[0].getSunriseTime() ||
            currentWeather.getTime() > dailyWeather[0].getSunsetTime()){
        printIcon(SUNRISE_ICON);
        iconX = 0;
    }
    else {
        printIcon(SUNSET_ICON);
        iconX = 64 + (SMALL_ICON >> 1);
    }

    tft.setCursor(iconX, cursorY);
    timeToLocal(dailyWeather[0].getSunsetTime());
    tft.println(displayOutput);
    iconX = 64 + (SMALL_ICON >> 1); //divide by 2
    tft.drawFastHLine(iconX, tft.getCursorY()+2, tft.width() - iconX - 2, TFT_WHITE);
    tft.setCursor(tft.getCursorX(), tft.getCursorY()+6);

    tft.drawFastVLine((tft.width() >> 1) - 1, tft.getCursorY() + (SMALL_ICON >> 1),
            tft.height() - tft.getCursorY() - 4, TFT_DARKGREY);
    //next day forecast
    iconY = tft.getCursorY();

    tft.setCursor(8,tft.getCursorY() + 8);
    printIcon(dailyWeather[1].getIcon());
    tft.setCursor(64+8,tft.getCursorY());
    printIcon(dailyWeather[2].getIcon());
    cursorY = tft.getCursorY();

    tft.setCursor(20, iconY, 2);
    tft.print("Mon");
    tft.setCursor(64+20, tft.getCursorY());
    tft.println("Tue");

    tft.setCursor(4,tft.getCursorY() + LARGE_ICON - 12);
    tft.print(dailyWeather[1].getTempHigh());
    tft.print("/");
    tft.print(dailyWeather[1].getTempLow());

    tft.setCursor(64+4,tft.getCursorY());
    tft.print(dailyWeather[2].getTempHigh());
    tft.print("/");
    tft.print(dailyWeather[2].getTempLow());

    tft.setCursor(2,tft.getCursorY(),1);
    tft.print("Rain:");
    tft.setCursor(tft.getCursorX()+2,tft.getCursorY());
    tft.print(dailyWeather[1].getPrecipProb());
    tft.print("%");

    tft.setCursor(64+2,tft.getCursorY());
    tft.print("Rain:");
    tft.setCursor(tft.getCursorX()+2,tft.getCursorY());
    tft.print(dailyWeather[2].getPrecipProb());
    tft.println("%");

    tft.setCursor(2,tft.getCursorY());
    tft.print(" Hum: 100%");
    tft.setCursor(64+2,tft.getCursorY());
    tft.print(" Hum: 100%");
}

void timeToLocal(time_t currentTime){
    TimeChangeRule *tcr;
    currentTime = tz.toLocal(currentTime, &tcr);
    snprintf(displayOutput, sizeof(displayOutput), "%02d:%02d ",
            hourFormat12(currentTime), minute(currentTime));
    if (isAM(currentTime)){
        strncat(displayOutput,"AM", 2);
    }
    else {
        strncat(displayOutput, "PM", 2);
    }
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

    Serial.println(temp);
    drawBmp(temp, tft.getCursorX(), tft.getCursorY());
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
