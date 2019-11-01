//rename to MiscSettings.h and add detail
//Settings for wifi, keep from git
#ifndef STASSID
#define STASSID "SSID"
#define STAPSK  "PSWD"
#endif

//darksky api key
#define API_KEY "key"

//Other vars
#define FORECAST_LOC "/42.3601,-71.0589" //example
#define UPDATE_INTERVAL 60UL
#define UPDATE_INTERVAL_MOTION 15UL //optional, stops rapid update with no motion
#define PIR_TIME 30UL    //time to keep display on, in seconds
//#define PIR_TIME 1UL * 60UL //for minutes, can change 1UL to xxUL
#define PIR_ON_TIME 5
#define PIR_OFF_TIME 12+11
//#define PIR_OFF_TIME_MORNING 1    //uncomment if off time is 12am or later

//rule vars
#define DAYLIGHT_RULE_CONFIG {"TZ", Second, Sun, Mar, 2, -0}
#define STANDARD_RULE_CONFIG {"TZ", First, Sun, Nov, 2, -0}
