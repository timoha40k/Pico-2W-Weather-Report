#include <lwip/err.h>
#include <lwip/ip_addr.h>
#include <lwip/opt.h>
#include "lwip/apps/sntp.h"
#include <pico/platform/panic.h>
#include <pico/time.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <time.h>

#include "pico/cyw43_arch.h"
#include "http_client.h"
#include "ntp_client.h"
#include "json_parser.h"

#include "sh1106.h"
#include "fonts.h"
#include "src/oled.h"
#include "src/network.h"
#include "wmo_icons.h"
#include "calendar.h"
#include "src/weather_cast.h"

void oled_draw_spritesheet_bitmap(uint8_t x, uint8_t y, uint8_t sprite_width, uint8_t sprite_height,
   uint8_t spritesheet_width, uint8_t spritesheet_height, uint8_t sprite_num, const uint8_t* spritesheet){
       uint8_t sheet_cols = spritesheet_width / sprite_width;
       uint8_t sheet_rows = spritesheet_height / sprite_height;

       uint8_t sprite_col = sprite_num % sheet_cols;
       uint8_t sprite_row = sprite_num / sheet_cols;

       uint8_t sheet_row_bytes = (spritesheet_width + 7) >> 3;
       uint8_t sprite_col_bytes = (sprite_col * sprite_width) >> 3;
        for (uint8_t h = 0; h < sprite_height; h++){
            uint8_t offset = (sprite_row * sprite_height + h) * sheet_row_bytes + sprite_col_bytes;
            oled_draw_bitmap(x, y + h, sprite_width, 1, (spritesheet + offset));
        }
}

int main(){
    stdio_init_all();
    sleep_ms(2000);//give serial time to connect
    printf("---------\n");

    rpi_start_oled();

    if (start_network_interface() != 0) {
        return 1;
    }

    connect_to_wifi();
    oled_fill(BLACK);

    http_response_t server_output = {0};

    get_weather_today(&server_output);

    float temperature[48];
    json_get_array_float(server_output.buf, "temperature_2m", temperature, sizeof(temperature)/sizeof(float));
    for (int i = 0; i < 48; i++){
        printf("%d: %.1f\n", i, temperature[i]);
        }
    float wmo_codes[48];
    json_get_array_float(server_output.buf, "weather_code", wmo_codes, 48);

    sleep_ms(100);

    //oled_print_float(64.5f, 6, 18, font3x5, &oled);
    struct tm* time;
    run_ntp_once(&time);

    uint8_t hour_now = time->tm_hour;
    //oled_print_float(25.72, 2, 60, 40, font3x5, &oled);
    //
    //calendar
    draw_time_date(time);

    //drawing boxes
    for(uint8_t i = 0; i < 4; i++){
        oled_draw_rect(i*32, 6, 32, 32);
    }

    oled_draw_vline(127, 6, 32);

    // weather information
    //oled_draw_bitmap(6, 7, WMO_ICONS_WIDTH, WMO_ICONS_HEIGHT, wmo_icons);
    /*oled_draw_spritesheet_bitmap(8, 8, 16, 16, WMO_ICONS_WIDTH, WMO_ICONS_HEIGHT, code_to_sprite(wmo_codes[0]), wmo_icons);
    oled_print_str_formating("%d ~C", 8, 25, font3x5, (int)temperature[hour_now]);
    oled_print_str_formating("%02d:00", 6, 32, font3x5, hour_now);*/

    uint8_t dt = 0;
    for (int i = 0; i <= 4; i++){
        uint8_t hour_next = hour_now + (i * 6);

        dt = (hour_next >= 24) ? 24 : 0;
        oled_print_str_formating("%d ~C", 8 + (32 * i), 25, font3x5, (int)temperature[hour_next]);
        oled_print_str_formating("%02d:00", 6 + (32 * i), 32, font3x5, hour_next - dt);
        oled_draw_spritesheet_bitmap(8 + (32 * i), 8, 16, 16, WMO_ICONS_WIDTH, WMO_ICONS_HEIGHT, code_to_sprite((uint8_t)wmo_codes[hour_next]), wmo_icons);
    }

    get_weather_week(&server_output);

    float wmo_week_code[7];
    json_get_array_float(server_output.buf, "weather_code", wmo_week_code, 7);
    float* max_temp_week = (float*)malloc(7);
    json_get_array_float(server_output.buf, "temperature_2m_max", max_temp_week, 7);
    float* min_temp_week = (float*)malloc(7);
    json_get_array_float(server_output.buf, "temperature_2m_min", min_temp_week, 7);

    float avg_temp_week[7];
    for (int i = 0; i < 7; i++){
        avg_temp_week[i] = (max_temp_week[i] + min_temp_week[i]) / 2.0f;
    }
    free(max_temp_week);
    free(min_temp_week);


    for (int i = 0; i < 7; i++){
        oled_draw_rect(i*18, 46, 18, 18);
        char temp[5];
        snprintf(temp, sizeof(temp), "%d ~C", (int)avg_temp_week[i]);
        oled_print_str(temp, i*18 + 2, 58, font3x5);
    }

    oled_update_screen();
    while(1) sleep_ms(2000);
    //for simplicity sake we'll assume that temperature[i] is temperature at i hour temperture[1] -> 01:00 etc

    cyw43_arch_deinit();

    sleep_ms(100);

    return 0;
}
