#ifndef CALENDAR_H
#define CALENDAR_H
#include "lib/sh1106/sh1106.h"
#include "ntp_client.h"

void draw_time_date(struct tm* time){
    oled_set_font3x5();
    oled_print_str_formating("%02d/%02d/%04d", 0, 0, font3x5, time->tm_mday, time->tm_mon + 1, time->tm_year + 1900);
    oled_print_str_formating("%02d:%02d", 54, 0, font3x5, time->tm_hour, time->tm_min);
}

#endif
