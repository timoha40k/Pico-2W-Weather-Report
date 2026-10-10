#ifndef OLED_HEAD
#define OLED_HEAD
#include <pico/stdlib.h>
#include <hardware/i2c.h>

#include "sh1106.h"
#include "fonts.h"
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
#endif
