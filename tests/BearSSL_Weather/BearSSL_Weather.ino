// Example of the different modes of the X.509 validation options
// in the WiFiClientBearSSL object
//
// Mar 2018 by Earle F. Philhower, III
// Released to the public domain
// Nov 2019 - Miles Young, modified url fetching

#include <ESP8266WiFi.h>
#include <WiFiClientSecure.h>
#include <TimeLib.h> //change from time.h, more specific

#ifndef STASSID
#define STASSID "abc"
#define STAPSK  "abc"
#endif

const char *ssid = STASSID;
const char *pass = STAPSK;

const char *   host = "api.darksky.net";
const uint16_t port = 443;
#define api_key "abc"
const char *   path = "/forecast/";
const char *   options = "/34,-118?exclude=minutely,hourly,flags,alerts,daily";

// Set time via NTP, as required for x.509 validation
void setClock() {
    configTime(3 * 3600, 0, "pool.ntp.org", "time.nist.gov");

    Serial.print("Waiting for NTP time sync: ");
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
}

// Try and connect using a WiFiClientBearSSL to specified host:port and dump HTTP response
void fetchURL(BearSSL::WiFiClientSecure *client, const char *host, const uint16_t port, const char *path) {
    if (!path) {
        path = "/";
    }

    ESP.resetFreeContStack();
    uint32_t freeStackStart = ESP.getFreeContStack();
    Serial.printf("Trying: %s:443...", host);
    client->connect(host, port);
    if (!client->connected()) {
        Serial.printf("*** Can't connect. ***\n-------\n");
        return;
    }
    Serial.printf("Connected!\n-------\n");
    client->write("GET ");
    client->write(path);
    client->write(" HTTP/1.0\r\nHost: ");
    client->write(host);
    client->write("\r\nUser-Agent: ESP8266\r\n");
    client->write("\r\n");
    uint32_t to = millis() + 5000;
    if (client->connected()) {
        do {
            char tmp[32];
            memset(tmp, 0, 32);
            int rlen = client->read((uint8_t*)tmp, sizeof(tmp) - 1);
            yield();
            if (rlen < 0) {
                break;
            }
            // Only print out first line up to \r, then abort connection
            char *nl = strchr(tmp, '\r');
            if (nl) {
                *nl = 0;
                Serial.print(tmp);
                break;
            }
            Serial.print(tmp);
        } while (millis() < to);
    }
    client->stop();
    uint32_t freeStackEnd = ESP.getFreeContStack();
    Serial.printf("\nCONT stack used: %d\n-------\n", freeStackStart - freeStackEnd);
}

void fetchInsecure() {
    Serial.println("===================\nInsecure");
    BearSSL::WiFiClientSecure client;
    client.setInsecure();
    uint32_t now = millis();

    //fetchURL(&client, host, port, path);
    Serial.printf("Trying: %s:443...", host);
    if (!client.connect(host, port)) {
        Serial.println("connection fail");
        client.stop();
        while(1){ ; } //halt
    }
    else {
        Serial.println("Connected");
        client.print(String("GET ") + path + api_key + options +
                " HTTP/1.1\r\n" +
                "Host: " + host + "\r\n" +
                "User-Agent: ESP8266\r\n" +
                "Connection: close\r\n\r\n");

        while (client.connected()) {
            String line = client.readStringUntil('\n');
            if (line == "\r") {
                break;
            }
        }
    }
    String line = client.readStringUntil('\n');
    Serial.println(line);

    uint32_t delta = millis() - now;
    Serial.printf("Time:%dms\n", delta);
}

void fetchCertAuthority() {
static const char digicert[] PROGMEM = R"EOF(
-----BEGIN CERTIFICATE-----
MIIDQTCCAimgAwIBAgITBmyfz5m/jAo54vB4ikPmljZbyjANBgkqhkiG9w0BAQsF
ADA5MQswCQYDVQQGEwJVUzEPMA0GA1UEChMGQW1hem9uMRkwFwYDVQQDExBBbWF6
b24gUm9vdCBDQSAxMB4XDTE1MDUyNjAwMDAwMFoXDTM4MDExNzAwMDAwMFowOTEL
MAkGA1UEBhMCVVMxDzANBgNVBAoTBkFtYXpvbjEZMBcGA1UEAxMQQW1hem9uIFJv
b3QgQ0EgMTCCASIwDQYJKoZIhvcNAQEBBQADggEPADCCAQoCggEBALJ4gHHKeNXj
ca9HgFB0fW7Y14h29Jlo91ghYPl0hAEvrAIthtOgQ3pOsqTQNroBvo3bSMgHFzZM
9O6II8c+6zf1tRn4SWiw3te5djgdYZ6k/oI2peVKVuRF4fn9tBb6dNqcmzU5L/qw
IFAGbHrQgLKm+a/sRxmPUDgH3KKHOVj4utWp+UhnMJbulHheb4mjUcAwhmahRWa6
VOujw5H5SNz/0egwLX0tdHA114gk957EWW67c4cX8jJGKLhD+rcdqsq08p8kDi1L
93FcXmn/6pUCyziKrlA4b9v7LWIbxcceVOF34GfID5yHI9Y/QCB/IIDEgEw+OyQm
jgSubJrIqg0CAwEAAaNCMEAwDwYDVR0TAQH/BAUwAwEB/zAOBgNVHQ8BAf8EBAMC
AYYwHQYDVR0OBBYEFIQYzIU07LwMlJQuCFmcx7IQTgoIMA0GCSqGSIb3DQEBCwUA
A4IBAQCY8jdaQZChGsV2USggNiMOruYou6r4lK5IpDB/G/wkjUu0yKGX9rbxenDI
U5PMCCjjmCXPI6T53iHTfIUJrU6adTrCC2qJeHZERxhlbI1Bjjt/msv0tadQ1wUs
N+gDS63pYaACbvXy8MWy7Vu33PqUXHeeE6V/Uq2V8viTO96LXFvKWlJbYK8U90vv
o/ufQJVtMVT8QtPHRh8jrdkPSHCa2XV4cdFyQzR1bldZwgJcJmApzyMZFo6IQ6XU
5MsI+yMRQ+hDKXJioaldXgjUkK642M4UwtBV8ob2xJNDd2ZhwLnoQdeXeGADbkpy
rqXRfboQnoZsG4q5WTP468SQvvG5
-----END CERTIFICATE-----
)EOF";

    Serial.println("===================\nCert");
    BearSSL::WiFiClientSecure client;
    BearSSL::X509List cert(digicert);
    client.setTrustAnchors(&cert);

    setClock();
    uint32_t now = millis();

    //fetchURL(&client, host, port, path);
    Serial.printf("Trying: %s:443...", host);
    if (!client.connect(host, port)) {
        Serial.println("connection fail");
        client.stop();
        while(1){ ; } //halt
    }
    else {
        Serial.println("Connected");
        client.print(String("GET ") + path + api_key + options +
                " HTTP/1.1\r\n" +
                "Host: " + host + "\r\n" +
                "User-Agent: ESP8266\r\n" +
                "Connection: close\r\n\r\n");

        while (client.connected()) {
            String line = client.readStringUntil('\n');
            if (line == "\r") {
                break;
            }
        }
    }
    String line = client.readStringUntil('\n');
    Serial.println(line);

    uint32_t delta = millis() - now;
    Serial.printf("Time:%dms\n", delta);
}

void setup() {
    Serial.begin(115200);
    Serial.println();
    Serial.println();

    // We start by connecting to a WiFi network
    Serial.print("Connecting to ");
    Serial.println(ssid);
    WiFi.mode(WIFI_STA);
    WiFi.begin(ssid, pass);

    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }
    Serial.println("");

    Serial.println("WiFi connected");
    Serial.println("IP address: ");
    Serial.println(WiFi.localIP());

    fetchInsecure();
    fetchCertAuthority();
    delay(5000);
    fetchCertAuthority(); //test if working on second try
    Serial.println("==============\nDone");
}

void loop() {
    // Nothing to do here
}
