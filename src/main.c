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
#include "wmo_icons.h"
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

    if (cyw43_arch_init()) {
        printf("init failed\n");
        return 1;
    }

    cyw43_arch_enable_sta_mode();
    printf("Connecting to %s...\n", WIFI_SSID);

    if (cyw43_arch_wifi_connect_timeout_ms(WIFI_SSID, WIFI_PASSWORD, CYW43_AUTH_WPA2_AES_PSK, 30000)) {
        printf("failed to connect\n");
        return 1;
    }
    printf("Connected!\n");

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

    i2c_init(i2c_default, 400000);
    gpio_set_function(PICO_DEFAULT_I2C_SDA_PIN, GPIO_FUNC_I2C);
    gpio_set_function(PICO_DEFAULT_I2C_SCL_PIN, GPIO_FUNC_I2C);
    gpio_pull_up(PICO_DEFAULT_I2C_SDA_PIN);
    gpio_pull_up(PICO_DEFAULT_I2C_SCL_PIN);

    sleep_ms(100); // I generally reccomend putting delay, otherwise it might cause strange behaviour

    /*const uint8_t sunny_32[]  = {
      0x00, 0x00, 0x00, 0x00,
      0x00, 0x02, 0x00, 0x00,
      0x00, 0x84, 0x20, 0x00,
      0x00, 0x80, 0x80, 0x00,
      0x00, 0x38, 0x00, 0x00,
      0x00, 0x88, 0x00, 0x00,
      0x02, 0x08, 0x00, 0x00,
      0x34, 0x16, 0x00, 0x00,
      0x08, 0x20, 0x00, 0x00,
      0x08, 0x80, 0x00, 0x00,
      0x0e, 0x00, 0x00, 0x00,
      0x80, 0x80, 0x00, 0x02,
      0x10, 0x80, 0x00, 0x00,
      0x20, 0x00, 0x00, 0x00,
      0x00, 0x00, 0x00, 0x00,
      0x00, 0x00, 0x00, 0x00,
      0x00, 0x00, 0x00, 0x00,
      0x00, 0x00, 0x00, 0x00,
      0x00, 0x00, 0x00, 0x00,
      0x00, 0x00, 0x00, 0x00,
      0x00, 0x00, 0x00, 0x00,
      0x00, 0x00, 0x00, 0x00,
      0x00, 0x00, 0x00, 0x00,
      0x00, 0x00, 0x00, 0x00,
      0x00, 0x00, 0x00, 0x00,
      0x00, 0x00, 0x00, 0x00,
      0x00, 0x00, 0x00, 0x00,
      0x00, 0x00, 0x00, 0x00,
      0x00, 0x00, 0x00, 0x00,
      0x00, 0x00, 0x00, 0x00,
      0x00, 0x00, 0x00, 0x00
    };

    uint8_t cloud[] = {
      0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xC0, 0x03, 0x20, 0x04,
      0x20, 0x08, 0x18, 0x10, 0x04, 0x20, 0x02, 0x20, 0x02, 0x20, 0x04, 0x10,
      0xF8, 0x0F, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};*/
    //oled_print_float(64.5f, 6, 18, font3x5, &oled);
    struct tm* time;
    run_ntp_once(&time);

    uint8_t hour_now = time->tm_hour;
    //oled_print_float(25.72, 2, 60, 40, font3x5, &oled);
    //
    oled_init();
    //calendar
    oled_set_font3x5();
    oled_print_str_formating("%02d/%02d/%04d", 0, 0, font3x5, time->tm_mday, time->tm_mon + 1, time->tm_year + 1900);
    oled_print_str_formating("%02d:%02d", 54, 0, font3x5, hour_now, time->tm_min);

    //drawing boxes
    for(uint8_t i = 0; i < 4; i++){
        oled_draw_rect(i*32, 6, 32, 32);
    }

    oled_draw_vline(127, 6, 32);

    // weather information
    //oled_draw_bitmap(6, 7, WMO_ICONS_WIDTH, WMO_ICONS_HEIGHT, wmo_icons);
    oled_draw_spritesheet_bitmap(8, 8, 16, 16, WMO_ICONS_WIDTH, WMO_ICONS_HEIGHT, code_to_sprite(wmo_codes[0]), wmo_icons);
    oled_print_str_formating("%d ~C", 8, 25, font3x5, (int)temperature[hour_now]);
    oled_print_str_formating("%02d:00", 6, 32, font3x5, hour_now);

    uint8_t dt = 0;
    for (int i = 1; i <= 4; i++){
        uint8_t hour_next = hour_now + (i * 6);

        dt = (hour_next >= 24) ? 24 : 0;
        oled_print_str_formating("%d ~C", 8 + (32 * i), 25, font3x5, (int)temperature[hour_next]);
        oled_print_str_formating("%02d:00", 6 + (32 * i), 32, font3x5, hour_next - dt);
        oled_draw_spritesheet_bitmap(8 + (32 * i), 8, 16, 16, WMO_ICONS_WIDTH, WMO_ICONS_HEIGHT, code_to_sprite((uint8_t)wmo_codes[hour_next]), wmo_icons);
    }

    oled_update_screen();
    while(1) sleep_ms(2000);
    //for simplicity sake we'll assume that temperature[i] is temperature at i hour temperture[1] -> 01:00 etc

    cyw43_arch_deinit();

    printf("test passed\n");
    sleep_ms(100);

    return 0;
}
