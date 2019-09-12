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
    #define API_KEY "xxx"
    #define FORECAST_LOC "/xx.xx,xx.xx"
    #define UPDATE_INTERVAL 15UL //every 15 minutes
    #define PIR_TIME 2UL //update interval for motion detection
    #define DAYLIGHT_RULE_CONFIG {"*DT", Second, Sun, Mar, 2, -XX0}
    #define STANDARD_RULE_CONFIG {"*ST", First, Sun, Nov, 2, -XX0}
*/
/* Uncomment in <ArduinoLibrary>/TFT_eSPI/User_Setup.h
    #define ST7735_Driver
    #define ST7735_GREENTAB //if colors wrong use different option
*/
//uncomment to print debug, from https://forum.arduino.cc/index.php?topic=46900.0
#define DEBUG true
#ifdef DEBUG
    #define DEBUG_PRINT(str)    Serial.print(str)
    #define DEBUG_PRINTLN(str)  Serial.println(str)
#else
    #define DEBUG_PRINT(str)
    #define DEBUG_PRINTLN(str)
#endif

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
#define pirPin D2
#define buttonPin D1
#define UNIX_MINUTE 60000UL //600000 = 60 Sec * 1000 uS

// Constant variables
const char *ssid = STASSID;
const char *password = STAPSK;
const char *host = "api.darksky.net";
const int httpsPort = 443;
const size_t capacity = JSON_ARRAY_SIZE(8) + JSON_OBJECT_SIZE(3) + JSON_OBJECT_SIZE(6) +
    JSON_OBJECT_SIZE(19) + 4*JSON_OBJECT_SIZE(38) + 4*JSON_OBJECT_SIZE(39) + 6180;

const String forecastType = "/forecast/";
//const String FORECAST_LOC //see define location
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
TimeChangeRule daylightRule = DAYLIGHT_RULE_CONFIG;
TimeChangeRule standardRule = STANDARD_RULE_CONFIG;
Timezone tz(daylightRule, standardRule);
bool haveSetup = false;

//display vars
uint16_t cursorX;
uint16_t cursorY;
char displayOutput[10];
volatile uint16_t rawBrightness;
volatile uint8_t newBrightness;
volatile bool brightnessInterrupt = false;
volatile bool displayOn = false;

//motion vars
unsigned long lastUpdateTime = 0;
unsigned long currentTime = 0;
volatile unsigned long pirTime = 0; //last updated
bool pirLast = LOW;
bool pirInput = LOW;
bool buttonLast = LOW;
bool buttonInput = LOW;

//function defs
void LED(bool led_output);
void updateBrightness();
void attachBrightness();
void detachBrightness();
void setBrightness(uint8_t newBrightness);
void setup();
void loop();
void startWeather();
bool getWeather();
void printWeatherDisplay();
void printTFTSpace(uint8_t i);
void timeToLocal(time_t currentTime);
void colorPrecip(int color);
void colorHumid(int color);
void printWater(const String typeWater, int water, uint8_t space);
void colorTemp(int color);
void printTemp(int tempH, int tempL, uint8_t space);
uint16_t rgbToHex(uint8_t red, uint8_t green, uint8_t blue);
bool printWeatherSerial();
void printIcon(const char *icon);
int checkWeatherIcon(const char *icon);
void clearScreen(int textSize);
void flashScreen();
void connectToWifi();
void disconnectWifi();

void LED(bool led_output){
    digitalWrite(LED_BUILTIN, !led_output); //need to flip the led
}

void updateBrightness(){
    rawBrightness = analogRead(A0);
    newBrightness = rawBrightness>>6; //change range
    if (newBrightness < 1){
        newBrightness = 1; //never be zero or else off
    }
    setBrightness(newBrightness);
}

void attachBrightness(){
    if (brightnessInterrupt == false){
        tftBrightness.attach_ms(500, updateBrightness); //call every 500ms
        brightnessInterrupt = true; //now active
    }
}
void detachBrightness(){
    if (brightnessInterrupt == true){
        tftBrightness.detach();
        brightnessInterrupt = false; //now not active
    }
}
void setBrightness(uint8_t newBrightness){
    analogWrite(pwmOut, newBrightness);
}

void setup() {
    pinMode(LED_BUILTIN, OUTPUT); //LED, GPIO 2, D4
    LED(HIGH);

#ifdef DEBUG
    Serial.begin(115200);
#endif
    DEBUG_PRINTLN();

    wifi_set_sleep_type(MODEM_SLEEP_T); //just turns off WiFi temporary

    analogWriteRange(pwmRange); //display brightness setup
    analogWriteFreq(pwmFreq);
    pinMode(pwmOut, OUTPUT);
    analogWrite(pwmOut, 1); //start at lowest brightness
    attachBrightness();

    tft.init();
    tft.setRotation(2);
    displayOn = true;
    clearScreen(2); //start at top, font size 2
    if ((tft.width() != DP_W) && (tft.height() != DP_H)){
        DEBUG_PRINT("DISPLAY FORMATTING NOT GUARANTEED");
    }

    if (!SPIFFS.begin()){
        DEBUG_PRINTLN("SPIFFS init failed...");
        while (1) { delay(1); } //stop program
    }
    DEBUG_PRINTLN("SPIFFS init");

#ifdef PIR_TIME
    pinMode(pirPin, INPUT);
#endif
    pirTime = millis();
    pinMode(buttonPin, INPUT);
    LED(LOW);

    connectToWifi();
    startWeather();
#ifdef DEBUG
    tft.setCursor(DP_W-8,0,1);
    tft.print("s");
#endif
}

void loop() {
    currentTime = millis();

#ifdef PIR_TIME
    pirInput = digitalRead(pirPin);

    if ((pirInput == HIGH) && (pirLast == LOW)){
       //turn on display...
        pirTime = millis();

        if (displayOn == false){
            DEBUG_PRINTLN("Turning display on");
            attachBrightness();
            setBrightness(8); //default
            startWeather();
            displayOn = true;
#ifdef DEBUG
            tft.setCursor(DP_W-8,0,1);
            tft.print("m");
#endif
        }
    }
    if (((pirTime + PIR_TIME*UNIX_MINUTE) <= currentTime) && (displayOn == true)){
        //not triggered for a while
        DEBUG_PRINTLN("Turning display off");
        flashScreen();
        detachBrightness();
        setBrightness(0);
        displayOn = false;
    }
#endif

    buttonInput = digitalRead(buttonPin);
    if ((buttonInput == HIGH) && (buttonLast == LOW)){
        startWeather();
#ifdef DEBUG
        tft.setCursor(DP_W-8,0,1);
        tft.print("b");
#endif
    }

    if ((lastUpdateTime + UPDATE_INTERVAL*UNIX_MINUTE) <= currentTime){
        startWeather();
#ifdef DEBUG
        tft.setCursor(DP_W-8,0,1);
        tft.print("a");
#endif
    }
    delay(50);
}

void startWeather(){
    bool output = getWeather();
    if (output == true){
        printWeatherDisplay();
        printWeatherSerial();
        pirTime = millis(); //update last time updated
        lastUpdateTime = millis();
    }
    else {
        DEBUG_PRINTLN("Parse FAILED");
        tft.println("Parse FAILED");
    }
}

bool getWeather() {
    clearScreen(2);
    tft.println("Powered by\nDark Sky");
    // Connect to remote server
    client.setInsecure(); //no https
    DEBUG_PRINT("connecting to ");
    DEBUG_PRINTLN(host);
    if (!client.connect(host, httpsPort)) {
        DEBUG_PRINTLN("connection failed");
        tft.println("connection fail");
        client.stop();
        return false;
    }
    else {
        DEBUG_PRINT("Getting forecast for: ");
        DEBUG_PRINTLN("https://" + String(host) +
                forecastType + API_KEY + FORECAST_LOC + forecastDetails);

        client.print(String("GET ") + forecastType + API_KEY +
                FORECAST_LOC + forecastDetails +
                " HTTP/1.1\r\n" +
                "Host: " + host + "\r\n" +
                "User-Agent: ESP8266\r\n" +
                "Connection: close\r\n\r\n");

        DEBUG_PRINTLN("request sent");
        while (client.connected()) {
            String line = client.readStringUntil('\n');
            if (line == "\r") {
                DEBUG_PRINTLN("headers received");
                tft.println("Got forecast");
                break;
            }
        }
    }

    //help from https://arduinojson.org/v6/assistant/
    DEBUG_PRINT("Created doc with size:"); DEBUG_PRINTLN(capacity);
    DynamicJsonDocument doc(capacity);
    DeserializationError error = deserializeJson(doc, client);

    if (error){
        DEBUG_PRINT("Parse failed - ");
        DEBUG_PRINTLN(error.c_str());
        tft.println("JSON parse failed");
        return false;
    }
    bool output = currentWeather.setupWeather(doc["currently"]);
    if (output == false){
        DEBUG_PRINTLN("Setup failed for current");
        return false;
    }
    for (int i=0; i<dailyWeatherSize; i++){
        output = dailyWeather[i].setupWeather(doc["daily"]["data"][i]);
        if (output == false){
            DEBUG_PRINTLN("Setup failed for daily");
            return false;
        }
    }

    DEBUG_PRINTLN("Done!");
    lastUpdateTime = millis();
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
    colorTemp(currentWeather.getTemp());
    tft.print(currentWeather.getTemp());
    tft.setTextColor(TFT_WHITE);
    tft.drawCircle(tft.getCursorX()+4, tft.getCursorY()+4, 2, TFT_WHITE); //degree symbol
    tft.setCursor(tft.getCursorX()+10, tft.getCursorY(), 2);
    tft.println("F");

    //Temp high/low
    tft.setCursor(DP_HALF_W, tft.getCursorY()+6, 2);
    printTemp(dailyWeather[0].getTempHigh(), dailyWeather[0].getTempLow(), 4);
    tft.println();

    //Rain/Humidity
    tft.setCursor(tft.getCursorX()+2, tft.getCursorY(), 2);
    printWater("Rain:", dailyWeather[0].getPrecipProb(), 4);
    tft.setCursor(DP_HALF_W+2, tft.getCursorY());
    printWater("RH:", dailyWeather[0].getHumidity(), 4);
    tft.println();
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
    tft.setTextColor(TFT_WHITE);
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
    printTemp(dailyWeather[1].getTempHigh(), dailyWeather[1].getTempLow(), 1);

    tft.setCursor(DP_HALF_W+4,tft.getCursorY());
    printTemp(dailyWeather[1].getTempHigh(), dailyWeather[1].getTempLow(), 1);
    tft.println();

    tft.setCursor(4,tft.getCursorY()-1,1);
    printWater("Rain:", dailyWeather[1].getPrecipProb(), 4);
    tft.setCursor(DP_HALF_W+2,tft.getCursorY());
    printWater("Rain:", dailyWeather[2].getPrecipProb(), 4);
    tft.println();

    tft.setCursor(4,tft.getCursorY());
    printWater("RH:", dailyWeather[2].getHumidity(), 4);
    tft.setCursor(DP_HALF_W+2,tft.getCursorY());
    printWater("RH:", dailyWeather[2].getHumidity(), 4);
    tft.println();
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
    if (color > 40){
        tft.setTextColor(rgbToHex(0, 255 - (color>>2), 255));
    }
    else {
        tft.setTextColor(TFT_WHITE);
    }
}

void colorHumid(int color){
    if (color > 70){
        tft.setTextColor(rgbToHex(0, 255 - (color>>3), 255));
    }
    else {
        tft.setTextColor(TFT_WHITE);
    }
}

void printWater(const String typeWater, int water, uint8_t space){
    colorPrecip(water);
    tft.print(typeWater);
    printTFTSpace(space);
    tft.print(water);
    tft.print("%");
    tft.setTextColor(TFT_WHITE);
}

void colorTemp(int color){
    if (color > 90){
        tft.setTextColor(TFT_RED);
    }
    else if (color > 75){
        tft.setTextColor(TFT_ORANGE);
    }
    else if (color > 50){
        tft.setTextColor(TFT_CYAN);
    }
    else if (color > 32){
        tft.setTextColor(TFT_LIGHTGREY);
    }
    else {
        tft.setTextColor(TFT_WHITE);
    }
}

void printTemp(int tempH, int tempL, uint8_t space){
    colorPrecip(tempH);
    tft.print(tempH);
    printTFTSpace(space);
    tft.setTextColor(TFT_WHITE);
    tft.print("/");
    printTFTSpace(space);
    colorTemp(tempL);
    tft.print(tempL);
    tft.setTextColor(TFT_WHITE);
}

//help from: http://www.barth-dev.de/online/rgb565-color-picker/
// and https://github.com/Bodmer/TFT_eSPI/blob/master/TFT_eSPI.h#L505
// RGB565 format is 5 for red, 6 for green, 5 for blue, fits in 16 bits
uint16_t rgbToHex(uint8_t red, uint8_t green, uint8_t blue){
    //((red & 0b11111000) << (11-3)) + ((green & 0b11111100) << (8-3-2)) +
    //    ((blue & 0b11111000) >> (3))
    return  ((red & 0b11111000) << (8)) + ((green & 0b11111100) << (3)) +
        ((blue & 0b11111000) >> (3));
}

bool printWeatherSerial(){
    char temp[120];
    if (currentWeather.getSetup()){
        DEBUG_PRINTLN("=================");
        snprintf(temp, sizeof(temp), "Temp: %i, Rain: %i Humidity: %i, time: %ld",
                currentWeather.getTemp(), currentWeather.getPrecipProb(),
                currentWeather.getHumidity(), currentWeather.getTime());
        DEBUG_PRINTLN(temp);
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
            DEBUG_PRINTLN(temp);
        }
        else {
            return false;
        }
    }
    DEBUG_PRINTLN("=================");
}

void printIcon(const char *icon){
    int iconNum = checkWeatherIcon(icon);

    char temp[25] = "/";
    strcat(temp, weatherIcon[iconNum]);
    strcat(temp, ".bmp");

    DEBUG_PRINTLN(temp);
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

void flashScreen(){
    tft.fillScreen(TFT_DARKGREY);
    tft.setCursor(0,0);
    tft.fillScreen(TFT_BLACK);
}

void clearScreen(int textSize){
    tft.fillScreen(TFT_BLACK);
    tft.setCursor(0,0,textSize);
}

void connectToWifi(){
    if (!haveSetup){
        tft.println("Powered by\nDark Sky");
        WiFi.forceSleepWake(); delay(1);
        DEBUG_PRINT("Connecting to ");
        DEBUG_PRINTLN(ssid);
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
            DEBUG_PRINTLN("Restarting");
            WiFi.disconnect(); delay(200);
            ESP.restart();
        }
        i++;
        if ( (i%5) == 0){
            DEBUG_PRINT("|");
            tft.print("|");
        }
        else {
            DEBUG_PRINT(".");
            tft.print(".");
        }
    }

    if (!haveSetup){
        tft.setCursor(0,cursorY,2);
        DEBUG_PRINT("\nConnected, IP address: ");
        DEBUG_PRINTLN(WiFi.localIP());
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
