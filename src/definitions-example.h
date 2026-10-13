#ifndef DEF_H
#define DEF_H
// IMORTANT!!!!!!!!!
// Change this filename to "definitions.h"
//
#define WIFI_SSID "your_wifi_name"
#define WIFI_PASSWORD "your_wifi_passsword"

#define WEATHER_TODAY "/v1/forecast?latitude=52.2298&longitude=21.0118&hourly=temperature_2m,weather_code&timezone=Europe%2FBerlin&past_days=0&forecast_days=2"
#define WEATHER_WEEK "/v1/forecast?latitude=52.2298&longitude=21.0118&daily=weather_code,temperature_2m_max,temperature_2m_min&timezone=Europe%2FBerlin"

#define BUTTON_LEFT_PIN 21
#define BUTTON_UP_PIN 20
#define BUTTON_DOWN_PIN 19
#define BUTTON_RIGHT_PIN 18

#endif
