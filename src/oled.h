#ifndef OLED_HEAD
#define OLED_HEAD
#include <pico/stdlib.h>
#include <hardware/i2c.h>
#include <stdint.h>
#include <time.h>

#include "sh1106.h"
#include "fonts.h"
#include "wmo_icons.h"
void rpi_start_oled(){
    //Oled init
    i2c_init(i2c_default, 400000);
    gpio_set_function(PICO_DEFAULT_I2C_SDA_PIN, GPIO_FUNC_I2C);
    gpio_set_function(PICO_DEFAULT_I2C_SCL_PIN, GPIO_FUNC_I2C);
    gpio_pull_up(PICO_DEFAULT_I2C_SDA_PIN);
    gpio_pull_up(PICO_DEFAULT_I2C_SCL_PIN);

    sleep_ms(200); // I generally reccomend putting delay, otherwise it might cause strange behaviour

    oled_init();
    oled_print_str("Booting...", 0, 0, font5x7);
    oled_update_screen();
    sleep_ms(100);
}

void draw_time_date(struct tm* time){
    oled_set_font3x5();
    oled_print_str_formating("%02d/%02d/%04d", 0, 0, font3x5, time->tm_mday, time->tm_mon + 1, time->tm_year + 1900);
    oled_print_str_formating("%02d:%02d", 54, 0, font3x5, time->tm_hour, time->tm_min);
}

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

void draw_weather_today_boxes(uint8_t hour_now, float temperature[], float wmo_codes[]){
    for (int i = 0; i <= 4; i++){
        oled_draw_rect(i*32, 6, 32, 32);
        uint8_t hour_next = hour_now + (i * 6);

        uint8_t dt = (hour_next >= 24) ? 24 : 0;
        oled_print_str_formating("%d ~C", 8 + (32 * i), 25, font3x5, (int)temperature[hour_next]);
        oled_print_str_formating("%02d:00", 6 + (32 * i), 32, font3x5, hour_next - dt);
        oled_draw_spritesheet_bitmap(8 + (32 * i), 8, 16, 16, WMO_ICONS_WIDTH, WMO_ICONS_HEIGHT, code_to_sprite((uint8_t)wmo_codes[hour_next]), wmo_icons);
    }
    oled_draw_vline(127, 6, 32);
}
#endif
