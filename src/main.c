#include <cyw43.h>
#include <cyw43_ll.h>
#include <hardware/gpio.h>
#include <lwip/err.h>
#include <lwip/ip_addr.h>
#include <lwip/opt.h>
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
#include "src/definitions.h"
#include "src/oled.h"
#include "src/network.h"
#include "wmo_icons.h"
#include "src/weather_cast.h"
#include "controller.h"

#define MICS_IN_SEC 1000000

static time_t base_time = 0;
static absolute_time_t base_at;

time_t get_now(){
    return base_time + absolute_time_diff_us(base_at, get_absolute_time()) / 1000000;
}

int main(){
    stdio_init_all();
    sleep_ms(2000);//give serial time to connect
    printf("---------\n");

    rpi_start_oled();

    const uint8_t buttonLeft = BUTTON_LEFT_PIN;
    const uint8_t buttonRight = BUTTON_RIGHT_PIN;
    const uint8_t buttonUp = BUTTON_UP_PIN;
    const uint8_t buttonDown = BUTTON_DOWN_PIN;
    init_controller();

    if (start_network_interface() != 0) {
        return 1;
    }

    if (connect_to_wifi() != 0){
        return 1;
    }
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


    /*for (int i = 0; i < 7; i++){
        oled_draw_rect(i*18, 46, 18, 18);
        char temp[5];
        snprintf(temp, sizeof(temp), "%d ~C", (int)avg_temp_week[i]);
        oled_print_str(temp, i*18 + 2, 58, font3x5);
        }*/

    time_t out_time;
    bool tried_sync = !run_ntp_once(&out_time);
    base_time = out_time;
    base_at = get_absolute_time();

    oled_update_screen();
    int min_last = -1;
    while(1) {
        int status = cyw43_tcpip_link_status(&cyw43_state, CYW43_ITF_STA); // checking if wifi is connected
        if (status == CYW43_LINK_UP){
            if (tried_sync && absolute_time_diff_us(base_at, get_absolute_time()) >  30LL * MICS_IN_SEC){
                printf("retrying to sync time...");
                if (run_ntp_once(&out_time)){
                    printf("time synced\n");
                    base_time = out_time;
                    base_at = get_absolute_time();
                    tried_sync = false;
                }
            }
            if (absolute_time_diff_us(base_at, get_absolute_time()) >  1800LL * MICS_IN_SEC && !tried_sync){
                if (run_ntp_once(&out_time)){
                    printf("time synced\n");
                    base_time = out_time;
                    base_at = get_absolute_time();
                    tried_sync = false;
                } else {
                    printf("tried syncing, failed");
                    tried_sync = true;
                }
            }
        } else {
            printf("No internet access, will sync later\n");
        }
        if (absolute_time_diff_us(base_at, get_absolute_time()) >  5LL * MICS_IN_SEC){
            time_t time_from_boot = get_now();
            struct tm t = *gmtime(&time_from_boot);

            if (min_last != t.tm_min){
                oled_fill(BLACK);

                draw_time_date(&t);
                draw_weather_today_boxes(t.tm_hour, temperature, wmo_codes);

            }

            min_last = t.tm_min;
        }
        enum Button button = read_controller();
        if (button != NO_PRESS){
            oled_set_color(BLACK);
            oled_draw_rect_filled(0, 39, OLED_W, 65-49);
            oled_set_color(WHITE);
            if (button == LEFT){
                oled_draw_rect_filled(0, 39, 16, 16);
            }
            if (button == UP){
                oled_draw_rect_filled(31, 39, 16, 16);
            }
            if (button == DOWN){
                oled_draw_rect_filled(62, 39, 16, 16);
            }
            if (button == RIGHT){
                oled_draw_rect_filled(93, 39, 16, 16);
            }
        }
        oled_update_screen();
        sleep_ms(100);
    }

    cyw43_arch_deinit();

    sleep_ms(100);

    return 0;
}
