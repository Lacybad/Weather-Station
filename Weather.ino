/*
    Weather with display
    Miles Young, 2019
 */

//Wifi
#include <ESP8266WiFi.h>
#include <WiFiClientSecure.h>
#include <ArduinoJson.h>
#include "src/weatherClass.h"
#include "MiscSettings.h" //D4=BUILTIN_LED,not RST
//display
//For TFT_eSPI, in User_Setup.h, set TFT_DC PIN_D0 and TFT_RST PIN_D6
#include <TFT_eSPI.h> //!!D6=RST!!,!!D0=A0/DC!!,D8=CS,D7=SDA,D5=SCL
#include <SPI.h>
#include "src/BMP_functions.h"
#include <Ticker.h> //timer interrupts for brightness
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
    #define ST7735_GREENTAB //if colors wrong use different option
*/
//display constants
#define DP_W 128        //display used, need to change if using different size
#define DP_HALF_W (DP_W >> 1)
#define DP_H 160
#define LARGE_ICON 44
#define SMALL_ICON 20
#define TIME_ICON 8     //size of time XX:XX XM
#define FS1 8           //font size height 1
#define FS2 16
#define FS4 26          //could be 32
//FS6=48, FS7=56/48, FS8=56/75  //either or
#define pwmRange 16     //16 bits
#define pwmFreq 1000      //1kHz frequency
#define pwmOut D3       //output pin for brightness

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
Ticker tftBrightness;

//weather variables
Weather currentWeather;
Weather dailyWeather[3]; //weather for today, tomorrow, and day+1
const int dailyWeatherSize = 3;
TimeChangeRule daylightRule = daylightRuleConfig;
TimeChangeRule standardRule = standardRuleConfig;
Timezone tz(daylightRule, standardRule);
bool haveSetup = false;

//display vars
uint16_t cursorX;
uint16_t cursorY;
char displayOutput[10];
volatile uint16_t rawBrightness;
volatile uint8_t newBrightness;

//function defs
void LED(bool led_output);
void updateBrightness();
void setup();
void loop();
bool getWeather();
void printWeatherDisplay();
void printTFTSpace(uint8_t i);
void timeToLocal(time_t currentTime);
void colorPrecip(int color);
uint16_t rgbToHex(uint8_t red, uint8_t green, uint8_t blue);
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

void updateBrightness(){
    rawBrightness = analogRead(A0);
    newBrightness = rawBrightness>>6; //change range
    if (newBrightness < 1){
        newBrightness = 1; //never be zero
    }
    analogWrite(pwmOut, newBrightness);
}

void setup() {
    pinMode(LED_BUILTIN, OUTPUT); //LED, GPIO 2, D4
    LED(HIGH);

    Serial.begin(115200);
    Serial.println();

    analogWriteRange(pwmRange); //display brightness setup
    analogWriteFreq(pwmFreq);
    pinMode(pwmOut, OUTPUT);
    analogWrite(pwmOut, 1); //start at lowest brightness
    tftBrightness.attach_ms(500, updateBrightness); //call every 500ms

    tft.init();
    tft.setRotation(2);
    clearScreen(2); //start at top, font size 2
    if ((tft.width() != DP_W) && (tft.height() != DP_H)){
        Serial.print("DISPLAY FORMATTING NOT GUARANTEED");
    }

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
    uint16_t iconY = 0;
    uint16_t iconX = 0;
    int tempVal = 0;

    //current weather
    tft.setCursor(tft.getCursorX()+1,tft.getCursorY());
    tft.print("Updated: ");
    timeToLocal(currentWeather.getTime());
    tft.println(displayOutput);
    cursorY = tft.getCursorY();
    tft.setCursor((DP_HALF_W - LARGE_ICON)>>1,cursorY);
    printIcon(currentWeather.getIcon());

    //Current Temp
    tft.setCursor(DP_HALF_W, cursorY, 4);
    tft.print(currentWeather.getTemp());
    tft.drawCircle(tft.getCursorX()+4, tft.getCursorY()+4, 2, TFT_WHITE); //degree symbol
    tft.setCursor(tft.getCursorX()+10, tft.getCursorY(), 2);
    tft.println("F");

    //Temp high/low
    tft.setCursor(DP_HALF_W, tft.getCursorY()+6, 2);
    tft.print(dailyWeather[0].getTempHigh());
    printTFTSpace(4);
    tft.print("/");
    printTFTSpace(4);
    tft.println(dailyWeather[0].getTempLow());

    //Rain/Humidity
    tft.setCursor(tft.getCursorX()+2, tft.getCursorY(), 2);
    tempVal = dailyWeather[0].getPrecipProb();
    colorPrecip(tempVal);
    tft.print("Rain:");
    printTFTSpace(2);
    tft.print(tempVal);
    tft.print("%");
    tft.setCursor(DP_HALF_W+2, tft.getCursorY());
    tempVal = dailyWeather[0].getHumidity();
    colorPrecip(tempVal);
    tft.print("RH:");
    printTFTSpace(2);
    tft.print(tempVal);
    tft.println("%");
    tft.setTextColor(TFT_WHITE);

    cursorY = tft.getCursorY();
    tft.setCursor(4,cursorY,1);
    timeToLocal(dailyWeather[0].getSunriseTime());
    tft.println(displayOutput);
    tft.setCursor(DP_HALF_W + (SMALL_ICON>>1)+2, cursorY);
    timeToLocal(dailyWeather[0].getSunsetTime());
    tft.println(displayOutput);
    cursorY = tft.getCursorY();

    tft.setCursor(DP_HALF_W-(SMALL_ICON>>1), cursorY-(SMALL_ICON>>1)); //divide 2
    if (currentWeather.getTime() < dailyWeather[0].getSunriseTime() ||
            currentWeather.getTime() > dailyWeather[0].getSunsetTime()){
        printIcon(SUNRISE_ICON);
        tft.drawLine(8, cursorY, DP_HALF_W - (SMALL_ICON>>1) - 4, cursorY, TFT_WHITE);
    }
    else {
        printIcon(SUNSET_ICON);
        tft.drawLine(DP_HALF_W+(SMALL_ICON>>1) + 4, cursorY, DP_W - 8, cursorY, TFT_WHITE);
    }

    tft.setCursor(tft.getCursorX(), tft.getCursorY()+6);

    //next day forecast
    cursorY = FS1 + FS4 + (FS2<<1) + FS1 + FS2 - (FS1>>1);
    tft.drawLine(DP_HALF_W - 1, cursorY, DP_HALF_W - 1, DP_H - FS1, TFT_DARKGREY);

    tft.setCursor(8,cursorY - (FS1>>1));
    printIcon(dailyWeather[1].getIcon());
    tft.setCursor(DP_HALF_W+8,tft.getCursorY());
    printIcon(dailyWeather[2].getIcon());

    tft.setCursor(20, cursorY - FS2, 2);
    tft.print(dayShortStr(weekday(dailyWeather[1].getTime())));
    tft.setCursor(DP_HALF_W+20, tft.getCursorY());
    tft.print(dayShortStr(weekday(dailyWeather[2].getTime())));

    tft.setCursor(4,cursorY + LARGE_ICON - FS1);
    tft.print(dailyWeather[1].getTempHigh());
    tft.print("/");
    tft.print(dailyWeather[1].getTempLow());

    tft.setCursor(DP_HALF_W+4,tft.getCursorY());
    tft.print(dailyWeather[2].getTempHigh());
    tft.print("/");
    tft.println(dailyWeather[2].getTempLow());

    tft.setCursor(2,tft.getCursorY(),1);
    tempVal = dailyWeather[1].getPrecipProb();
    colorPrecip(tempVal);
    tft.print("Rain:");
    printTFTSpace(4);
    tft.print(tempVal);
    tft.print("%");
    tft.setCursor(DP_HALF_W+2,tft.getCursorY());
    tempVal = dailyWeather[2].getPrecipProb();
    colorPrecip(tempVal);
    tft.print("Rain:");
    printTFTSpace(4);
    tft.print(tempVal);
    tft.println("%");

    tft.setCursor(2,tft.getCursorY());
    tempVal = dailyWeather[1].getHumidity();
    colorPrecip(tempVal);
    tft.print("RH:");
    printTFTSpace(4);
    tft.print(tempVal);
    tft.print("%");
    tft.setCursor(DP_HALF_W+2,tft.getCursorY());
    tempVal = dailyWeather[2].getHumidity();
    colorPrecip(tempVal);
    tft.print("RH:");
    printTFTSpace(4);
    tft.print(tempVal);
    tft.println("%");
    tft.setTextColor(TFT_WHITE);

    haveSetup = true;
}

void printTFTSpace(uint8_t i){
    tft.setCursor(tft.getCursorX()+i,tft.getCursorY());
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

void colorPrecip(int color){
    if (color > 75){
        tft.setTextColor(TFT_BLUE); //is lighter dark blue
    }
    else if (color > 50){ //is light blue
        tft.setTextColor(TFT_CYAN);
    }
    else {
        tft.setTextColor(TFT_WHITE);
    }
}

//help from: http://www.barth-dev.de/online/rgb565-color-picker/
// RGB565 format is 5 for red, 6 for green, 5 for blue, fits in 16 bits
uint16_t rgbToHex(uint8_t red, uint8_t green, uint8_t blue){
    //((red & 0b11111000) << (11-3)) + ((green & 0b11111100) << (8-3)) +
    //    ((blue & 0b11111000) >> (3))
    return  ((red & 0b11111000) << (8)) + ((green & 0b11111100) << (5)) +
        ((blue & 0b11111000) >> (3));
}

bool printWeatherSerial(){
    char temp[40];
    if (currentWeather.getSetup()){
        Serial.println("=================");
        snprintf(temp, sizeof(temp), "Temp: %i, Rain: %i Humidity: %i, time: %ld",
                currentWeather.getTemp(), currentWeather.getPrecipProb(),
                currentWeather.getHumidity(), currentWeather.getTime());
        Serial.println(temp);
    }
    else {
        return false;
    }
    for (int i=0; i<dailyWeatherSize; i++){
        if (dailyWeather[i].getSetup()){
            snprintf(temp, sizeof(temp), "%i Temp H: %i, Temp L: %i, Rain: %i Humidity: %i, time: %ld, sunrise time: %ld, sunset time: %ld",
                i, dailyWeather[i].getTempHigh(), dailyWeather[i].getTempLow(),
                dailyWeather[i].getPrecipProb(), dailyWeather[i].getHumidity(),
                dailyWeather[i].getTime(), dailyWeather[i].getSunriseTime(),
                dailyWeather[i].getSunsetTime());
            Serial.println(temp);
        }
        else {
            return false;
        }
    }
    Serial.println("=================");
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
    if (!haveSetup){
        tft.println("Powered by\nDark Sky");
        WiFi.forceSleepWake(); delay(1);
        Serial.print("Connecting to ");
        Serial.println(ssid);
        tft.println("Connecting to:");
        tft.println(ssid);
    }
    else {
        tft.setCursor(tft.getCursorX()+1,0);
        tft.print("Dark Sky Updating");
    }

    WiFi.persistent(false); delay(1);
    WiFi.mode(WIFI_STA); delay(1);
    WiFi.begin(ssid, password);

    uint8_t i = 0;
    tft.setCursor(0,152,1);
    cursorY = tft.getCursorY();
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        if ( (i%20) == 0) {
            tft.drawString("                    ", 0, cursorY);
            tft.setCursor(0,152,1);
        }
        else if (i == 50){
            Serial.println("Restarting");
            WiFi.disconnect(); delay(200);
            ESP.restart();
        }
        i++;
        if ( (i%5) == 0){
            Serial.print("|");
            tft.print("|");
        }
        else {
            Serial.print(".");
            tft.print(".");
        }
    }

    if (!haveSetup){
        tft.setCursor(0,cursorY,2);
        Serial.print("\nConnected, IP address: ");
        Serial.println(WiFi.localIP());
        tft.print("\nIP: ");
        tft.println(WiFi.localIP());
    }
}

void disconnectWifi(){
    WiFi.disconnect();
    delay(1);
    WiFi.mode(WIFI_OFF);
    delay(1);
    WiFi.forceSleepBegin();
    delay(1);
}
