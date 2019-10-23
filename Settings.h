//rename to MiscSettings.h and add detail
//Settings for wifi, keep from git
#ifndef STASSID
#define STASSID "SSID"
#define STAPSK  "PSWD"
#endif

//darksky api key
#define API_KEY "key"

//Other vars
#define FORECAST_LOC "lat/long"
#define UPDATE_INTERVAL 20UL
#define PIR_TIME 1UL    //time to keep display on
#define PIR_ON_TIME 5
#define PIR_OFF_TIME 12+11

//rule vars
#define DAYLIGHT_RULE_CONFIG {"TZ", Second, Sun, Mar, 2, -0}
#define STANDARD_RULE_CONFIG {"TZ", First, Sun, Nov, 2, -0}
