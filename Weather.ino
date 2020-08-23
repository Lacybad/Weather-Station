/*
    Weather with display
    Miles Young, 2019
 */

//Wifi
#include <ESP8266WiFi.h>
#include <WiFiClientSecure.h>
#include <ArduinoJson.h>
#include "src/weatherClass.h"
//#include "src/cert.h"       //location for cert in memory
#include "Settings.h"       //settings file
//display //drivers - greentab, blacktab+inversion(orange)
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

/* include a MiscSettings.h or Settings.h file for these defs
STASSID, STAPSK, API_KEY, FORECAST_LOC, UPDATE_INTERVAL, PIR_TIME,
PIR_ON_TIME, PIR_OFF_TIME, DAYLIGHT_RULE_CONFIG, STANDARD_RULE_CONFIG
*/
//uncomment to print debug, from https://forum.arduino.cc/index.php?topic=46900.0
//hint - when debugging, change the times to trigger things faster...
//#define DEBUG
#ifdef DEBUG
    //#define SHOW_BRIGHTNESS
    //#define SHOW_MOTION
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
#define FSX1 6          //font size width 1
#define FS2 16
#define FSX2 8
#define FS4 26          //could be 32
//FS6=48, FS7=56/48, FS8=56/75  //either or
#define displayUpdate 1000  //in ms
#define pwmShift 9          //also change brightness max
#define pwmJump 8           //min difference
#define pwmRange (1<<pwmShift)-1 //max range
#define pwmFreq 1000      //1kHz frequency
#define pwmOut D3       //output pin for brightness
#define pirPin D2
#define buttonPin D1
#define UNIX_SECOND 1000UL //1000 uS
#define UNIX_MINUTE UNIX_SECOND*60UL

// Constant variables
const char *ssid = STASSID;
const char *password = STAPSK;
const char *host = "api.openweathermap.org";
#ifdef USE_CERT
    const int httpsPort = 443;
    const String httpsProtocol="HTTP/1.1";
#else
    const int httpsPort = 80; //is http
    const String httpsProtocol="HTTP/1.0";
#endif
const size_t capacity = 9*JSON_ARRAY_SIZE(1) + JSON_ARRAY_SIZE(8) + 17*JSON_OBJECT_SIZE(4)
    + 9*JSON_OBJECT_SIZE(6) + 9*JSON_OBJECT_SIZE(14) + 2000;

const String forecastType = "/data/2.5/onecall";
//const String FORECAST_LOC //see define location
const String forecastDetails = "exclude=minutely,hourly,flags,alerts";
const String forecastStr = forecastType + "?" + FORECAST_LOC + "&" + forecastDetails +
"&units=" + UNITS + "&appid=" + API_KEY;

#define SUNRISE_ICON "sunrise"
#define SUNSET_ICON "sunset"

//global variables
TFT_eSPI tft = TFT_eSPI(); //start library
Ticker tftBrightness;

//weather variables
Weather currentWeather;
#define dailyWeatherSize 3
Weather dailyWeather[dailyWeatherSize]; //weather for today, tomorrow, and day+1
int16_t timezoneOffset = 0;
bool haveSetup = false;

//display vars
uint16_t cursorX;
uint16_t cursorY;
char displayOutput[10];
volatile uint16_t rawBrightness; //10 bit number
volatile uint16_t newBrightness;
volatile uint16_t oldBrightness = 1;
volatile bool displayOn = false;

//motion vars
volatile unsigned long lastUpdateTime = 0;
volatile unsigned long currentTime = 0;
volatile unsigned long currentLocalTime = 0; //if not getting ntp time
volatile uint8_t currentHour = 0;
volatile unsigned long pirTime = 0; //last updated
bool pirLast = LOW;
bool pirInput = LOW;
bool buttonLast = LOW;
bool buttonInput = LOW;

//function defs
void LED(bool led_output);
void updateBrightness();
void displayOnOff();
void setBrightness(uint8_t newBrightness);
void setup();
void loop();
void startWeather();
void setClock();
bool getWeather();
void printWeatherDisplay();
inline void printTFTSpace(uint8_t i);
void timeLocalStr(time_t getTime);
void timeStr(time_t getTime);
void setLocalTime(time_t getLocalTime);
void colorPrecip(int color);
void colorPrecipIntensity(float color);
void colorHumid(int color);
void printPrecip(const String typeWater, int water, uint8_t space);
void printPrecipIntensity(float waterAmt, uint8_t space);
void printHumid(const String typeWater, int water, uint8_t space);
void colorTemp(int color);
void printTempCenter(int tempH, int tempL, uint8_t space, uint8_t fontSize,
        uint8_t textWidth);
void printTemp(int tempH, int tempL, uint8_t space);
inline uint16_t rgbToHex(uint8_t red, uint8_t green, uint8_t blue);
bool printWeatherSerial();
void printIcon(const char *icon);
int checkWeatherIcon(const char *icon);
inline void clearScreen(int textSize);
void flashScreen();
void connectToWifi();
void disconnectWifi();

//flips the LED, is a wrapper function due to ESP8266 having inverse LED
void LED(bool led_output){
    digitalWrite(LED_BUILTIN, !led_output); //need to flip the led
}

//updates the current brightness, changes slowly
void updateBrightness(){
    rawBrightness = analogRead(A0);
    newBrightness = rawBrightness>>(10-pwmShift); //change range: 0-2^(10-x) range

    if (newBrightness < 1){
        newBrightness = 1; //never be zero or else off
    }
    //jump function
    if (abs(newBrightness-oldBrightness) > pwmJump){
        oldBrightness = newBrightness; //jump if large difference
    }
    if (newBrightness > oldBrightness){
        oldBrightness++;
    }
    else if (newBrightness < oldBrightness){
        oldBrightness--;
    }
    setBrightness(oldBrightness);
    //else - no change
#ifdef SHOW_BRIGHTNESS
    DEBUG_PRINT("b: ");
    DEBUG_PRINT(newBrightness);
#endif
}

//turns the display off (along with brightness) or on
void displayOnOff(){
    if (displayOn == false){
        DEBUG_PRINTLN("Turning display on");
        tftBrightness.attach_ms(displayUpdate, updateBrightness); //call every 500ms
        flashScreen(); //clear screen
        updateBrightness();
        displayOn = true;
#ifdef DEBUG
        LED(true);
#endif
    }
    else {
        DEBUG_PRINTLN("Turning display off");
        tftBrightness.detach();
        flashScreen(); //clear screen
        setBrightness(0);
        displayOn = false;
#ifdef DEBUG
        LED(false);
#endif
    }
}

//force sets the brightness of the display
void setBrightness(uint8_t newBrightness){
    analogWrite(pwmOut, newBrightness);
}

//initial setup of the program
void setup() {
    pinMode(LED_BUILTIN, OUTPUT); //LED, GPIO 2, D4
    LED(HIGH);

#ifdef DEBUG
    Serial.begin(115200);
    DEBUG_PRINTLN("\nStartup");
    DEBUG_PRINT("Brightness range: ");
    Serial.println(pwmRange, BIN);
#endif

    wifi_set_sleep_type(MODEM_SLEEP_T); //just turns off WiFi temporary

    analogWriteRange(pwmRange); //display brightness setup
    analogWriteFreq(pwmFreq);
    pinMode(pwmOut, OUTPUT);
    analogWrite(pwmOut, 1); //start at lowest brightness
    displayOn = false;
    displayOnOff();
    tft.init();
    tft.setRotation(2);

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
    pirTime = millis(); //for right now, blank value
    pinMode(buttonPin, INPUT);
    LED(LOW);

    connectToWifi();
    startWeather();
#ifdef DEBUG
    tft.setCursor(DP_W-8,0,1);
    tft.print("s");
    currentHour = hour(now());
    DEBUG_PRINT("Hour: "); DEBUG_PRINTLN(currentHour);
#endif
}

//Checks for motion, button press, and update time
void loop() {
    currentTime = millis(); //time since started

#ifdef PIR_TIME
    currentHour = hour(now());
#ifdef PIR_OFF_TIME_MORNING
    if (currentHour >= PIR_ON_TIME || currentHour < PIR_OFF_TIME){ //only during day
#else
    if (currentHour >= PIR_ON_TIME && currentHour < PIR_OFF_TIME){ //only during day
#endif
        pirInput = digitalRead(pirPin);

        if ((pirInput == HIGH) && (pirLast == LOW)){
           //turn on display...
            pirTime = millis();

            if (displayOn == false){
                displayOnOff();
                printWeatherDisplay(); //just turned on...
                DEBUG_PRINTLN("Display Motion Updated...");
            }
#ifdef UPDATE_INTERVAL_MOTION
            if ((lastUpdateTime + UPDATE_INTERVAL_MOTION*UNIX_MINUTE) <= currentTime){
#else
            if ((lastUpdateTime + UPDATE_INTERVAL*UNIX_MINUTE) <= currentTime){
#endif
                DEBUG_PRINTLN("Motion update");
                startWeather(); //only get new data after a period
            }
#ifdef DEBUG
            tft.setCursor(DP_W-8,0,1);
            tft.print("m");
#ifdef SHOW_MOTION
            DEBUG_PRINT("M ");
#endif
#endif
        }
        pirLast == pirInput; //move to last
    }
    else {
        delay(450); //longer sleep at night
    }
#endif
    //turn off if no motion for a while, always (no sleep hours)
    if ((displayOn == true) && ((pirTime + PIR_TIME*UNIX_SECOND) <= currentTime)){
        DEBUG_PRINTLN("Auto turnoff");
        displayOnOff(); //not triggered for a while
    }

    //button function
    buttonInput = digitalRead(buttonPin);
    if ((buttonInput == HIGH) && (buttonLast == LOW)){
        DEBUG_PRINTLN("Button Pressed");
        if (displayOn == false){
            displayOnOff();
        }
        startWeather(); //force get new data
#ifdef DEBUG
        tft.setCursor(DP_W-8,0,1);
        tft.print("b");
#endif
    }
    buttonLast = buttonInput; //update

    if ((lastUpdateTime + UPDATE_INTERVAL*UNIX_MINUTE) <= currentTime){
        DEBUG_PRINTLN("Auto updated");
        //flashScreen(); //don't clear screen yet
        startWeather();
#ifdef DEBUG
        tft.setCursor(DP_W-8,0,1);
        tft.print("a");
#endif
    }
    delay(50);
}

//checks the parsing of the weather, updates display and update time
void startWeather(){
    bool output = getWeather();
    if (output == true){
        if(haveSetup){
            DEBUG_PRINTLN("----------\nFlash display");
            flashScreen(); //flash screen to refresh colors, not on boot
        }

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

//set time via NTP, for tls (X.509) certificate
//From ESP8266 example HTTPSRequestCSCertAxTLS
void setClock() {
    DEBUG_PRINT("Setting time using SNTP: ");
    //timezone, daylightoffset, servers
    configTime(0, 0, "pool.ntp.org", "time.nist.gov"); //utc time
    time_t now = time(nullptr);
    while (now < 8 * 3600 * 2) {
        delay(500);
        DEBUG_PRINT(".");
        now = time(nullptr);
    }
    DEBUG_PRINTLN("");
    setLocalTime(now);
    //struct tm timeinfo;
    //gmtime_r(&now, &timeinfo);
    //DEBUG_PRINT("Current time: ");
    //DEBUG_PRINT(asctime(&timeinfo));
}

//gets the weather from api, formats json
bool getWeather() {
    tft.fillRect(0,DP_H-FS1, DP_W, FS1, TFT_BLACK); //clear line
    tft.setCursor(0,DP_H-FS1,1);
    tft.println("Src: OpenWeatherMap");

#ifdef USE_CERT
    DEBUG_PRINTLN("Setting up cert");
    BearSSL::WiFiClientSecure client;
    BearSSL::X509List cert(digicert);
    client.setTrustAnchors(&cert);
    //client.setInsecure(); //no https

    setClock(); //set clock, update when getting new as drift occurs on uC
#else
    // Connect to remote server
    WiFiClient client;
#endif

    DEBUG_PRINT("connecting to ");
    DEBUG_PRINTLN(host);
    if (!client.connect(host, httpsPort)) {
        DEBUG_PRINTLN("Connection failed");
        tft.fillRect(0,DP_H-FS1, DP_W, FS1, TFT_BLACK); //clear line
        tft.setCursor(0,DP_H-FS1,1);
#ifdef USE_CERT
        tft.println("Connection fail\nCould be cert fail");
#else
        tft.println("Connection fail");
#endif
        client.stop();
        if(displayOn == true){
            setBrightness(5);
        }
        delay(60*1000); //stop for 60 minutes
        ESP.restart(); //restart to avoid loop
        return false; //should never get here
    }
    else {
        DEBUG_PRINT("Getting forecast for: ");
        DEBUG_PRINTLN("https://" + String(host) + forecastStr);

        client.print(String("GET ") + forecastStr + " " + httpsProtocol + "\r\n" +
                "Host: " + host + "\r\n" +
                "User-Agent: ESP8266\r\n" +
                "Connection: close\r\n\r\n");

        DEBUG_PRINTLN("request sent");
        while (client.connected() || client.available()) {
            if (client.available()){
                String line = client.readStringUntil('\n');
                //DEBUG_PRINTLN(line); //print header
                if (line == "\r") {
                    DEBUG_PRINTLN("headers received");
                    tft.fillRect(0,DP_H-FS1, DP_W, FS1, TFT_BLACK); //clear line
                    tft.setCursor(0,DP_H-FS1,1);
                    tft.print("Got forecast");
                    break;
                }
            }
        }
    }

    //help from https://arduinojson.org/v6/assistant/
    DEBUG_PRINT("Created doc with size:"); DEBUG_PRINT(capacity); DEBUG_PRINT(" -> ");
    DynamicJsonDocument doc(capacity);
    tft.println("..");
    DeserializationError error = deserializeJson(doc, client.readStringUntil('\n'));
    tft.println(".....");
    DEBUG_PRINT("Parse Status: ");

    if (error){
        DEBUG_PRINT("Parse failed - ");
        DEBUG_PRINTLN(error.c_str());
        tft.println("JSON parse failed");
        delay(1*1000);
        return false;
    }
    else {
        //check if can get any key
        DEBUG_PRINTLN("Parse succeeded");
        const char *error = doc["timezone"];
        if (error == NULL){
            DEBUG_PRINT("Checking if message exists: ");
            const char *msg = doc["message"];
            if (msg != NULL){
                DEBUG_PRINTLN(msg);
                clearScreen(1);
                tft.println(msg);
            }
            return false;
        }
    }

    DEBUG_PRINTLN("Getting Data");
    bool output = currentWeather.setupWeather(doc["current"], false);
    if (output == false){
        DEBUG_PRINTLN("Setup failed for current");
        return false;
    }
    else {
        DEBUG_PRINTLN("Setup current");
    }

    timezoneOffset = doc["timezone_offset"];
    //setTime(currentWeather.getTime());
    setLocalTime(currentWeather.getTime()); //set time
    timeLocalStr(currentWeather.getTime());
    DEBUG_PRINT(timezoneOffset); DEBUG_PRINT(" "); DEBUG_PRINT(currentWeather.getTime());
        DEBUG_PRINT(" -> "); DEBUG_PRINTLN(displayOutput);

    //is a work-around - sometimes does not get the icon string correctly after wakeup
    DEBUG_PRINT(" "); DEBUG_PRINTLN(currentWeather.getIcon());

    for (int i=0; i<dailyWeatherSize; i++){
        output = dailyWeather[i].setupWeather(doc["daily"][i], true);
        if (output == false){
            DEBUG_PRINTLN("Setup failed for daily");
            return false;
        }
        else {
            DEBUG_PRINT("Setup daily ");
        }
    }

    DEBUG_PRINTLN("\nDone!");
    lastUpdateTime = millis();
    client.stop();
    return true;
}

//outputs weather to the display
void printWeatherDisplay(){
    clearScreen(1);
    uint16_t iconY = 0;
    uint16_t iconX = 0;
    int tempVal = 0;
    int tempHigh = 0;
    int tempLow = 0;
    if(!currentWeather.getSetup()){
        tft.println("Not setup");
        return;
    }

    //current weather
    tft.setCursor(tft.getCursorX()+1,tft.getCursorY());
    tft.print("Updated: ");
    timeLocalStr(currentWeather.getTime());
    tft.println(displayOutput);
    cursorY = tft.getCursorY();
    tft.setCursor((DP_HALF_W - LARGE_ICON)>>1,cursorY+1);
    printIcon(currentWeather.getIcon());

    //Current Temp
    tft.setCursor(DP_HALF_W, cursorY, 4);
    //colorTemp(currentWeather.getTemp());
    tft.print(currentWeather.getTemp());
    tft.setTextColor(TFT_WHITE);
    tft.drawCircle(tft.getCursorX()+4, tft.getCursorY()+4, 2, TFT_WHITE); //degree symbol
    tft.setCursor(tft.getCursorX()+10, tft.getCursorY(), 2);
    tft.println("F");
    tft.setCursor(DP_HALF_W, tft.getCursorY()+(FS1>>1)+1, 1);
    printPrecip("Rain:", currentWeather.getPrecipProb(), 2);

    //Temp high/low
    tft.setCursor(DP_HALF_W, tft.getCursorY()+FS1-1, 2);
    printTemp(dailyWeather[0].getTempHigh(), dailyWeather[0].getTempLow(), 4);
    tft.println();

    //Rain/Humidity
    tft.setCursor(tft.getCursorX(), tft.getCursorY()-(FS1>>2), 2);
    printPrecip("Rain:", dailyWeather[0].getPrecipProb(), 4);
    tft.setCursor(DP_HALF_W+6, tft.getCursorY());
#ifdef PRECIP_INSTY
    printPrecipIntensity(dailyWeather[0].getPrecipAmt(), 4);
#else
    printHumid("RH:", dailyWeather[0].getHumidity(), 4);
#endif
    tft.println();
    tft.setTextColor(TFT_WHITE);

    cursorY = tft.getCursorY();
    tft.setCursor(4,cursorY,1);
    timeLocalStr(dailyWeather[0].getSunriseTime());
    tft.println(displayOutput);
    tft.setCursor(DP_HALF_W + (SMALL_ICON>>1)+2, cursorY);
    timeLocalStr(dailyWeather[0].getSunsetTime());
    tft.println(displayOutput);
    cursorY = tft.getCursorY();

    tft.setCursor(DP_HALF_W-(SMALL_ICON>>1), cursorY-(SMALL_ICON>>1)); //divide 2
    if (currentWeather.getTime() < dailyWeather[0].getSunriseTime() ||
            currentWeather.getTime() > dailyWeather[0].getSunsetTime()){
        printIcon(SUNRISE_ICON); //is after sunset or before sunrise
        tft.drawLine(4, cursorY, DP_HALF_W - (SMALL_ICON>>1) - 4, cursorY, TFT_WHITE);
    }
    else {
        printIcon(SUNSET_ICON); //is after sunrise
        tft.drawLine(DP_HALF_W+(SMALL_ICON>>1) + 4, cursorY, DP_W - 4, cursorY, TFT_WHITE);
    }

    tft.setCursor(tft.getCursorX(), tft.getCursorY()+FS1-1);

    //next day forecast
    tft.setTextColor(TFT_WHITE);
    cursorY = FS1 + FS4 + (FS2<<1) + FS2 + FS1;
    tft.drawLine(DP_HALF_W - 1, cursorY - (FS1>>2), DP_HALF_W - 1, DP_H - (FS1>>2), TFT_DARKGREY);

    tft.setCursor(8,cursorY - (FS1>>1)+1);
    printIcon(dailyWeather[1].getIcon());
    tft.setCursor(DP_HALF_W+8,tft.getCursorY());
    printIcon(dailyWeather[2].getIcon());

    tft.setCursor(20, cursorY - FS2, 2);
    tft.print(dayShortStr(weekday(dailyWeather[1].getTime())));
    tft.setCursor(DP_HALF_W+20, tft.getCursorY());
    tft.print(dayShortStr(weekday(dailyWeather[2].getTime())));

    tft.setCursor(0,cursorY + (LARGE_ICON - (FS1>>1)));
    printTempCenter(dailyWeather[1].getTempHigh(), dailyWeather[1].getTempLow(), 1,
            FSX2, DP_HALF_W);

    tft.setCursor(DP_HALF_W,tft.getCursorY());
    printTempCenter(dailyWeather[2].getTempHigh(), dailyWeather[2].getTempLow(), 1,
            FSX2, DP_HALF_W);
    tft.println();

    tft.setCursor(4,tft.getCursorY()-1,1);
    printPrecip("Rain:", dailyWeather[1].getPrecipProb(), 4);
    tft.setCursor(DP_HALF_W+2,tft.getCursorY());
    printPrecip("Rain:", dailyWeather[2].getPrecipProb(), 4);
    tft.println();

    tft.setCursor(4,tft.getCursorY());
#ifdef PRECIP_INSTY
    printPrecipIntensity(dailyWeather[1].getPrecipAmt(), 4);
#else
    printHumid("RH:", dailyWeather[2].getHumidity(), 4);
#endif
    tft.setCursor(DP_HALF_W+2,tft.getCursorY());
#ifdef PRECIP_INSTY
    printPrecipIntensity(dailyWeather[2].getPrecipAmt(), 4);
#else
    printHumid("RH:", dailyWeather[2].getHumidity(), 4);
#endif
    tft.println();
    tft.setTextColor(TFT_WHITE);

    haveSetup = true;
}

//prints a variable size space
inline void printTFTSpace(uint8_t i){
    tft.setCursor(tft.getCursorX()+i,tft.getCursorY());
}

//converts UTC to timezone
void timeLocalStr(time_t getTime){
    getTime = getTime + timezoneOffset;
    //timeStr(getTime);
    snprintf(displayOutput, sizeof(displayOutput), "%02d:%02d ",
            hourFormat12(getTime), minute(getTime));
    if (isAM(getTime)){
        strncat(displayOutput,"AM", 2);
    }
    else {
        strncat(displayOutput, "PM", 2);
    }
}

void timeStr(time_t getTime){
    snprintf(displayOutput, sizeof(displayOutput), "%02d:%02d ",
            hourFormat12(getTime), minute(getTime));
    if (isAM(getTime)){
        strncat(displayOutput,"AM", 2);
    }
    else {
        strncat(displayOutput, "PM", 2);
    }
}

//sets the current local time
void setLocalTime(time_t getLocalTime){
    currentLocalTime = getLocalTime + timezoneOffset;
    setTime(currentLocalTime);
#ifdef debug
    DEBUG_PRINTLN("Set local time to:");
    timeStr(getLocalTime);
    DEBUG_PRINT(displayOutput);
#endif
}

//changes the color of rain to stand out more
void colorPrecip(int color){
    if (color > 40){
        tft.setTextColor(rgbToHex(0, 255 - (color>>2), 255));
    }
    else {
        tft.setTextColor(TFT_WHITE);
    }
}

//changes the color of rain to stand out more
void colorPrecipIntensity(float color){
    if (color >= 0.01){
        uint16_t blueAmt = color*100; //get rid of decimal place
        if (blueAmt > 255){
            blueAmt = 255;
        }
        tft.setTextColor(rgbToHex(0, 255 - blueAmt, 255));
    }
    else {
        tft.setTextColor(TFT_WHITE);
    }
}

//changes humidity to stand out more
void colorHumid(int color){
    if (color > 70){
        tft.setTextColor(rgbToHex(0, 255 - (color>>3), 255));
    }
    else {
        tft.setTextColor(TFT_WHITE);
    }
}

//prints rain format (rain: xx%)
void printPrecip(const String typeWater, int water, uint8_t space){
    colorPrecip(water);
    tft.print(typeWater);
    printTFTSpace(space);
    tft.print(water);
    tft.print("%");
    tft.setTextColor(TFT_WHITE);
}

//prints rain in inches
void printPrecipIntensity(float waterAmt, uint8_t space){
    colorPrecipIntensity(waterAmt);
    tft.print(waterAmt);
    printTFTSpace(space);
    tft.print(PRECIP_UNIT);
    tft.setTextColor(TFT_WHITE);
}

//prints relative humidity format (RH: xx%)
void printHumid(const String typeWater, int water, uint8_t space){
    tft.setTextColor(TFT_LIGHTGREY);
    tft.print(typeWater);
    printTFTSpace(space);
    tft.print(water);
    tft.print("%");
    tft.setTextColor(TFT_WHITE);
}

//changes with color of temperature
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

//prints each temperature in middle
void printTempCenter(int tempH, int tempL, uint8_t space, uint8_t fontSize,
        uint8_t textWidth){
    cursorX = (space<<1) + fontSize; // includes slash and space

    if (tempH < 0) cursorX += fontSize; //neg temp
    if (abs(tempH) >= 100) cursorX += fontSize; //if over 100
    if (abs(tempH) >= 10) cursorX += fontSize; //if over 10
    cursorX += fontSize; //always, 1's place

    if (tempL < 0) cursorX += fontSize; //neg temp
    if (abs(tempL) >= 100) cursorX += fontSize;
    if (abs(tempL) >= 10) cursorX += fontSize;
    cursorX += fontSize; //always

    if (cursorX >= DP_HALF_W) cursorX = 0; //is overflowing
    else cursorX = (DP_HALF_W - cursorX) >> 1; //cut in half difference

    tft.setCursor(tft.getCursorX() + cursorX,tft.getCursorY()); //set cursor
    printTemp(tempH, tempL, space); //print
}

//prints temperature format ( xx / xx )
void printTemp(int tempH, int tempL, uint8_t space){
    //colorPrecip(tempH); //hard to read, commented out
    tft.print(tempH);
    printTFTSpace(space);
    tft.setTextColor(TFT_WHITE);
    tft.print("/");
    printTFTSpace(space);
    //colorTemp(tempL);
    tft.print(tempL);
    tft.setTextColor(TFT_WHITE);
}

//help from: http://www.barth-dev.de/online/rgb565-color-picker/
// and https://github.com/Bodmer/TFT_eSPI/blob/master/TFT_eSPI.h#L505
// RGB565 format is 5 for red, 6 for green, 5 for blue, fits in 16 bits
inline uint16_t rgbToHex(uint8_t red, uint8_t green, uint8_t blue){
    //((red & 0b11111000) << (11-3)) + ((green & 0b11111100) << (8-3-2)) +
    //    ((blue & 0b11111000) >> (3))
    return  ((red & 0b11111000) << (8)) + ((green & 0b11111100) << (3)) +
        ((blue & 0b11111000) >> (3));
}

//prints debug information
bool printWeatherSerial(){
#ifdef DEBUG
    char temp[120];
    if (currentWeather.getSetup()){
        DEBUG_PRINTLN("=================");
        snprintf(temp, sizeof(temp), "Temp: %i, Rain: %i Humidity: %i, time: %ld (",
                currentWeather.getTemp(), currentWeather.getPrecipProb(),
                currentWeather.getHumidity(), currentWeather.getTime());
        DEBUG_PRINT(temp);
        timeLocalStr(currentWeather.getTime());
        DEBUG_PRINT(displayOutput); DEBUG_PRINTLN(")");
    }
    else {
        return false;
    }
    for (int i=0; i<dailyWeatherSize; i++){
        if (dailyWeather[i].getSetup()){
            snprintf(temp, sizeof(temp), "%i Temp H: %i, Temp L: %i, Rain: %i %.3f Humidity: %i, time: %ld, sunrise time: %ld, sunset time: %ld ",
                i, dailyWeather[i].getTempHigh(), dailyWeather[i].getTempLow(),
                dailyWeather[i].getPrecipProb(), dailyWeather[i].getPrecipAmt(),
                dailyWeather[i].getHumidity(),
                dailyWeather[i].getTime(), dailyWeather[i].getSunriseTime(),
                dailyWeather[i].getSunsetTime());
            DEBUG_PRINT(temp);
        timeLocalStr(dailyWeather[i].getTime());
        DEBUG_PRINT(" ("); DEBUG_PRINT(displayOutput); DEBUG_PRINT(")");
        timeLocalStr(dailyWeather[i].getSunriseTime());
        DEBUG_PRINT(" ("); DEBUG_PRINT(displayOutput); DEBUG_PRINT(")");
        timeLocalStr(dailyWeather[i].getSunsetTime());
        DEBUG_PRINT(" ("); DEBUG_PRINT(displayOutput); DEBUG_PRINT(")");
        DEBUG_PRINTLN();
        }
        else {
            return false;
        }
    }
    DEBUG_PRINTLN("=================");
#endif
}

//prints icon from storage
void printIcon(const char *icon){
    DEBUG_PRINT(icon); DEBUG_PRINT(" ");

    char temp[25] = "/";
    strcat(temp, icon); //always has a file
    strcat(temp, ".bmp");

    if (!SPIFFS.exists(temp)){
       DEBUG_PRINTLN("Using default na");
       strcpy(temp, "/na.bmp");
    }

    DEBUG_PRINT(temp); DEBUG_PRINT(" ---- ");

    drawBmp(temp, tft.getCursorX(), tft.getCursorY()); //just in case
}

//flash screen to remove stuck pixels
void flashScreen(){
    tft.fillScreen(TFT_BLACK);
    delay(5);
    tft.fillScreen(TFT_DARKGREY);
    delay(5);
    tft.setCursor(0,0);
    tft.fillScreen(TFT_BLACK);
}

//erase screen
inline void clearScreen(int textSize){
    tft.fillScreen(TFT_BLACK);
    tft.setCursor(0,0,textSize);
}

//quick connection to wifi
void connectToWifi(){
    if (!haveSetup){
        tft.println("Powered by\nOpenWeatherMap");
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
    tft.setCursor(0,DP_H-FS1,1);
    cursorY = tft.getCursorY();
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        if ( (i%20) == 0) {
            tft.fillRect(0,cursorY, DP_W, FS1, TFT_BLACK); //clear line
            tft.setCursor(0,DP_H-FS1,1);
        }
        if (i == 20*3){ //go by 20 for display, if > 75 + slow wifi = never connect
            tft.setCursor(0,DP_H-FS1,1); //make sure in same position
            tft.print("Restart");
            DEBUG_PRINTLN("Restarting");
            WiFi.disconnect(); delay(500);
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

//quick disconnect from wifi
void disconnectWifi(){
    WiFi.disconnect();
    delay(1);
    WiFi.mode(WIFI_OFF);
    delay(1);
    WiFi.forceSleepBegin();
    delay(1);
}

