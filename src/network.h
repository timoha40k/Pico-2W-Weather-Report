#ifndef NETWORK_H
#define NETWORK_H
#include "pico/cyw43_arch.h"
#include "sh1106.h"
#include "definitions.h"

int start_network_interface(){
    if (cyw43_arch_init()) {
        printf("init failed\n");
        oled_print_str("Failed to initialise cyw43", 0, 0, font5x7);
        oled_update_screen();
        sleep_ms(5000);
        return 1;
    }
    return 0;
}

int connect_to_wifi(){
    cyw43_arch_enable_sta_mode();
    printf("Connecting to %s...\n", WIFI_SSID);
    oled_fill(BLACK);
    oled_print_str_formating("Connecting to: ", 0, 0, font5x7);
    oled_print_str_formating("%s...", 0, 10, font5x7, WIFI_SSID);
    oled_update_screen();

    if (cyw43_arch_wifi_connect_timeout_ms(WIFI_SSID, WIFI_PASSWORD, CYW43_AUTH_WPA2_AES_PSK, 30000)) {
        printf("failed to connect\n");
        oled_fill(BLACK);
        oled_print_str_formating("Failed to connect to: ", 0, 0, font5x7);
        oled_print_str_formating("%s", 0, 10, font5x7, WIFI_SSID);
        oled_update_screen();
        sleep_ms(2000);
        return 1;
    }
    printf("Connected!\n");
    return 0;
}
#endif
