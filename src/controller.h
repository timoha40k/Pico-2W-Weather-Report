#ifndef CONTROLLER_H
#define CONTROLLER_H
#include "src/definitions.h"
#include <stdint.h>
#include <hardware/gpio.h>
enum Button {LEFT, RIGHT, UP, DOWN, NO_PRESS};
void init_controller(){
    gpio_init(BUTTON_LEFT_PIN);
    gpio_init(BUTTON_RIGHT_PIN);
    gpio_init(BUTTON_UP_PIN);
    gpio_init(BUTTON_DOWN_PIN);
    gpio_set_dir(BUTTON_LEFT_PIN, GPIO_IN);
    gpio_set_dir(BUTTON_RIGHT_PIN, GPIO_IN);
    gpio_set_dir(BUTTON_UP_PIN, GPIO_IN);
    gpio_set_dir(BUTTON_DOWN_PIN, GPIO_IN);
    gpio_pull_up(BUTTON_LEFT_PIN);
    gpio_pull_up(BUTTON_RIGHT_PIN);
    gpio_pull_up(BUTTON_UP_PIN);
    gpio_pull_up(BUTTON_DOWN_PIN);
}

enum Button read_controller(){
    if (!gpio_get(BUTTON_LEFT_PIN)) // pressed pin returns 0
        return LEFT;
    if (!gpio_get(BUTTON_RIGHT_PIN))
        return RIGHT;
    if (!gpio_get(BUTTON_UP_PIN))
        return UP;
    if (!gpio_get(BUTTON_DOWN_PIN))
        return DOWN;
    return NO_PRESS;
}
#endif
