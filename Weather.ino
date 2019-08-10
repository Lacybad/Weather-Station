/*
    Weather with display - using TLS
    Miles Young, 2019
    Modified from HTTP over TLS example, with root CA Certificate
    Original Author: Ivan Grokhotkov, 2017 
 */

#define USING_AXTLS
#include <time.h>
#include <ESP8266WiFi.h>
#include "MiscSettings.h"
#include "CACert.h"

// force use of AxTLS (BearSSL is now default)
#include <WiFiClientSecureAxTLS.h>
using namespace axTLS;

/* include a MiscSettings.h file for these defs
    #ifndef STASSID
    #define STASSID "WiFi"
    #define STAPSK  "Password"
    #endif
    #define apikey "xxx"
*/

const char* ssid = STASSID;
const char* password = STAPSK;

// Constant variables
const char* host = "api.darksky.net";
const int httpsPort = 443;
const String forecastType = "/forecast/";
const String forecastLoc = "/44.0039944,-123.0630231";
const String forecastDetails = "?exclude=minutely,hourly,daily,flags";

// Root certificate used by api.github.com.
// Defined in "CACert" tab.
extern const unsigned char caCert[] PROGMEM;
extern const unsigned int caCertLen;

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored  "-Wdeprecated-declarations"
WiFiClientSecure client;
#pragma GCC diagnostic pop

void LED(bool led_output){
    digitalWrite(LED_BUILTIN, !led_output);
}

void setup() {
    pinMode(LED_BUILTIN, OUTPUT); //LED, GPIO 2, D4
    LED(HIGH);

    Serial.begin(115200);
    Serial.println();
    if (WiFi.SSID() != ssid) {
        //save time, no need to reconfigure if already configured
        Serial.print("connecting to ");
        Serial.println(ssid);
        WiFi.mode(WIFI_STA);
        WiFi.begin(ssid, password);
        WiFi.persistent(true);
        WiFi.setAutoConnect(true);
        WiFi.setAutoReconnect(true);
    }
    else {
        Serial.print("connected to ");
        Serial.println(ssid);
    }

    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }
    Serial.println("");
    Serial.println("WiFi connected");
    Serial.println("IP address: ");
    Serial.println(WiFi.localIP());

    // Synchronize time useing SNTP. This is necessary to verify that
    // the TLS certificates offered by the server are currently valid.
    Serial.println(caCertLen);
    Serial.print("Setting time using SNTP");
    configTime(8 * 3600, 0, "pool.ntp.org", "time.nist.gov");
    time_t now = time(nullptr);
    while (now < 8 * 3600 * 2) {
        delay(500);
        Serial.print(".");
        now = time(nullptr);
    }
    Serial.println("");
    struct tm timeinfo;
    gmtime_r(&now, &timeinfo);
    Serial.print("Current time: ");
    Serial.print(asctime(&timeinfo));

    // Load root certificate in DER format into WiFiClientSecure object
    bool res = client.setCACert_P(caCert, caCertLen);
    if (!res) {
        Serial.println("Failed to load root CA certificate!");
        while (true) {
            yield();
        }
    }
    LED(LOW);
    getWeather();
}

void loop() {
    delay(100);
}

void getWeather() {
    // Connect to remote server
    Serial.print("connecting to ");
    Serial.println(host);
    if (!client.connect(host, httpsPort)) {
        Serial.println("connection failed");
        return;
    }

    // Verify validity of server's certificate
/*    Serial.println("Verifying cert");
    if (client.verifyCertChain(host)) {
        Serial.println("Server certificate verified");
    } else {
        Serial.println("ERROR: certificate verification failed!");
        return;
    } */

    Serial.print("Getting forecast for: ");
    Serial.println(forecastType + apikey + forecastLoc + forecastDetails);

    client.print(String("GET ") + forecastType + apikey + 
            forecastLoc +
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
    String line = client.readStringUntil('\n');
/*    if (line.startsWith("{\"state\":\"success\"")) {
        Serial.println("esp8266/Arduino CI successfull!");
    } else {
        Serial.println("esp8266/Arduino CI has failed");
    } */
    Serial.println("reply was:");
    Serial.println("==========");
    Serial.println(line);
    Serial.println("==========");
    Serial.println();
}
