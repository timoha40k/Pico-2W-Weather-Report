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
#include "src/weather_cast.h"

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

    sleep_ms(100);

    i2c_init(i2c_default, 400000);
    gpio_set_function(PICO_DEFAULT_I2C_SDA_PIN, GPIO_FUNC_I2C);
    gpio_set_function(PICO_DEFAULT_I2C_SCL_PIN, GPIO_FUNC_I2C);
    gpio_pull_up(PICO_DEFAULT_I2C_SDA_PIN);
    gpio_pull_up(PICO_DEFAULT_I2C_SCL_PIN);

    sleep_ms(100); // I generally reccomend putting delay, otherwise it might cause strange behaviour

    SH1106* oled = oled_init();

    const uint8_t sunny_32[]  = {
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
      0xF8, 0x0F, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};

    for(uint8_t i = 0; i < 7; i++){
        oled_draw_rect(i*32, 6, 32, 32, oled);
    }
    oled_draw_vline(127, 6, 32, oled);
    oled_draw_bitmap(0, 7, 31, 32, sunny_32, oled);
    oled_draw_bitmap(40, 7, 16, 16, cloud, oled);
    //oled_print_str("4*C", 6, 16, font3x5, &oled);
    //oled_draw_bitmap(6, 16, 8, 3, ch_a, &oled);
    //oled_print_ch('~', 3, 16, font5x7, &oled);
    //oled_print_str("Some text 12349", 5, 40, font5x7, &oled);
    oled_set_font3x5(oled);
    //oled_print_float(64.5f, 6, 18, font3x5, &oled);
    struct tm* time;
    run_ntp_once(&time);
    printf("got ntp response: %02d/%02d/%04d %d:%02d:%02d\n", time->tm_mday, time->tm_mon + 1, time->tm_year + 1900,
           time->tm_hour, time->tm_min, time->tm_sec);

    uint8_t hour_now = time->tm_hour;
    //oled_print_float(25.72, 2, 60, 40, font3x5, &oled);
    //calendar
    oled_print_str_formating("%02d/%02d/%04d", 0, 0, font3x5, oled, time->tm_mday, time->tm_mon + 1, time->tm_year + 1900);
    oled_print_str_formating("%02d:%02d", 54, 0, font3x5, oled, hour_now, time->tm_min);

    // weather information
    oled_print_str_formating("%d ~C", 8, 25, font3x5, oled, (int)temperature[hour_now]);
    oled_print_str_formating("%02d:00", 6, 32, font3x5, oled, hour_now);

    uint8_t dt = 0;
    for (int i = 1; i <= 4; i++){
        uint8_t hour_next = hour_now + (i * 6);

        dt = (hour_next >= 24) ? 24 : 0;
        oled_print_str_formating("%d ~C", 8 + (32 * i), 25, font3x5, oled, (int)temperature[hour_next]);
        oled_print_str_formating("%02d:00", 6 + (32 * i), 32, font3x5, oled, hour_next - dt);
    }

    oled_update_screen(oled);
    while(1) sleep_ms(2000);
    //for simplicity sake we'll assume that temperature[i] is temperature at i hour temperture[1] -> 01:00 etc

    cyw43_arch_deinit();

    printf("test passed\n");
    sleep_ms(100);

    return 0;
}
