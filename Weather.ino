/*
    Weather with display
    Miles Young, 2019
    Modified from HTTP over TLS example, with root CA Certificate
    Original Author: Ivan Grokhotkov, 2017 
 */

#include <time.h>
#include <ESP8266WiFi.h>
#include <WiFiClientSecure.h>
#include <Arduino_JSON.h>
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
const String forecastDetails = "?exclude=minutely,hourly,daily,flags";

BearSSL::WiFiClientSecure client;

void LED(bool led_output){
    digitalWrite(LED_BUILTIN, !led_output); //need to flip the led
}

void setup() {
    pinMode(LED_BUILTIN, OUTPUT); //LED, GPIO 2, D4
    LED(HIGH);

    Serial.begin(115200);
    Serial.print("\nConnecting to ");
    Serial.println(ssid);
    WiFi.mode(WIFI_STA);
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
    
    char jsonInput[400] = client.readStringUntil('\n');
    JSONVar weatherJson = JSON.parse(jsonInput);

    if (weatherJson.typeof(weatherJson) == "undefined"){
        Serial.println("Parse failed");
        return;
    }

    if (weatherJson.hasOwnProperty("summary")){
        Serial.println((const char*) weatherJson["summary"]);
    }
}
