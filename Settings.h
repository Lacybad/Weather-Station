#define _MISCSETTINGS_H
#ifdef _MISCSETTINGS_H
#include "MiscSettings.h" //use personal settings
#else
//if file is not defined
#ifndef STASSID
#define STASSID "SSID"
#define STAPSK  "PSWD"
#endif

//darksky api key
#define API_KEY "abcdef0123456789"
#define FORECAST_LOC "/40.7828687,-73.9675438" //example, is NYC

//timezone vars
#define DAYLIGHT_RULE_CONFIG {"EDT", Second, Sun, Mar, 2, -(4*60)}
#define STANDARD_RULE_CONFIG {"EST", First, Sun, Nov, 2, -(5*60)}
#endif

//Other vars
#define UPDATE_INTERVAL 2*60UL
#define UPDATE_INTERVAL_MOTION 20UL //optional, stops rapid update with no motion
#define PIR_TIME 30UL    //time to keep display on, in seconds
//#define PIR_TIME 1UL * 60UL //for minutes, can change 1UL to xxUL
#define PIR_ON_TIME 5
#define PIR_OFF_TIME 12+11
//#define PIR_OFF_TIME_MORNING 1    //uncomment if off time is 12am or later

//humidity or not
#define PRECIP_INSTY //use precip intensity, else use humidity
//units
#define UNITS "us" //units using
#define PRECIP_UNIT "in" //to print out
