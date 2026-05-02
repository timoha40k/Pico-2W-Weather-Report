#ifndef _WEATHER_CAST_H
#define _WEATHER_CAST_H
#include "pico/cyw43_arch.h"
#include "http_client.h"
#include <stdio.h>

#define HOST "api.open-meteo.com"
//#define WEATHER_TODAY "/v1/forecast?latitude=52.2298&longitude=21.0118&hourly=temperature_2m,weather_code&timezone=Europe%2FBerlin&past_days=0&forecast_days=2"
//#define WEATHER_WEEK "/v1/forecast?latitude=52.2298&longitude=21.0118&daily=weather_code,temperature_2m_max,temperature_2m_min&timezone=Europe%2FBerlin"
void get_weather_today(http_response_t* server_output){
    http_request_t req = {0};
    req.hostname = HOST;
    req.url = WEATHER_TODAY;
    req.callback_arg = (void*)server_output;
    req.headers_fn = http_client_header_printf;
    req.recv_fn = http_client_receive_print;
    printf("%s %s", HOST, WEATHER_TODAY);
    int result = http_client_request_sync(cyw43_arch_async_context(), &req);
    if (result != ERR_OK){
        printf("Couldn't resolve request err code %d", result);
        cyw43_arch_deinit();
        sleep_ms(100);
    }
}
#endif
