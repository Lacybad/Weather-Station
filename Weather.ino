/*
    Weather with display
    Miles Young, 2019
    Modified from HTTP over TLS example, with root CA Certificate
    Original Author: Ivan Grokhotkov, 2017 
 */

#include <time.h>
#include <ESP8266WiFi.h>
#include <WiFiClientSecure.h>
#include <ArduinoJson.h>
#include "MiscSettings.h"
#include "CACert.h"

/* include a MiscSettings.h file for these defs
    #ifndef STASSID
    #define STASSID "WiFi"
    #define STAPSK  "Password"
    #endif
    #define apikey "xxx"
    #define forecastLoc "/xx.xx,xx.xx"
*/

const char* ssid = STASSID;
const char* password = STAPSK;

// Constant variables
const char* host = "api.darksky.net";
const int httpsPort = 443;
const String forecastType = "/forecast/";
//const String forecastLoc //see define location
const String forecastDetails = "?exclude=minutely,hourly,flags";

BearSSL::WiFiClientSecure client;

void LED(bool led_output){
    digitalWrite(LED_BUILTIN, !led_output); //need to flip the led
}

void setup() {
    pinMode(LED_BUILTIN, OUTPUT); //LED, GPIO 2, D4
    LED(HIGH);

    Serial.begin(115200);
    WiFi.forceSleepWake(); delay(1);
    Serial.print("\nConnecting to ");
    Serial.println(ssid);
    WiFi.persistent(false); delay(1);
    WiFi.mode(WIFI_STA); delay(1);
    WiFi.begin(ssid, password);

    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }
    Serial.print("Connected, IP address: ");
    Serial.println(WiFi.localIP());
    
    LED(LOW);
    getWeather();
}

void loop() {
    delay(100);
}
    
const size_t capacity = JSON_ARRAY_SIZE(8) + JSON_OBJECT_SIZE(3) + JSON_OBJECT_SIZE(6) + JSON_OBJECT_SIZE(19) + 4*JSON_OBJECT_SIZE(38) + 4*JSON_OBJECT_SIZE(39) + 6170;

void getWeather() {
    // Connect to remote server
    client.setInsecure(); //no https
    Serial.print("connecting to ");
    Serial.println(host);
    if (!client.connect(host, httpsPort)) {
        Serial.println("connection failed");
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
    float latitude = doc["latitude"];
    Serial.println(latitude);

    JsonObject currently = doc["currently"];
    float currently_temperature = currently["temperature"];

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
