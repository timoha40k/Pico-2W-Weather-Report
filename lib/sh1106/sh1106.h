#ifndef SH1106_H
#define SH1106_H

#include "fonts.h"
#include <ctype.h>
#include <pico/stdlib.h>
#include <hardware/i2c.h>
#include <pico/binary_info.h>
#include <stdarg.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <math.h>

#define SH1106_I2C_ADDR 0x3C
#define OLED_W 128
#define OLED_H 64

#define I2C_MACRO i2c_default //might want to change that according to your wiring

#define CMD_REGISTER 0x00
#define DATA_REGISTER 0x40

#define DISPLAY_OFF 0xAE
#define DISPLAY_ON 0xAF

#define LOWER_COL_ADDR 0x02 //the x beggining of the screen
#define HIGHER_COL_ADDR 0x10

#define PAGE_ADDR 0xB0
#define COM_DIRECTION 0xC8 //0xC0 for reversed vertically

#define START_LINE 0x40 // the y beggining of the screen

#define SET_CONTRAST_CONTROL_MODE 0x81

#define CONTRAST_VAL 0xFF //how bright is the oled

#define SEGMENT_REMAP 0xA1

#define SET_NORMAL_DISPLAY 0xA6
#define SET_REVERSE_DISPLAY 0xA7

#define SET_MULTIPLEX_RATION_MODE 0xA8
#define MULTIPLEX_RATION_VAL 0x3F // how many rows will display show

#define ENTIRE_DISPLAY_ON 0xA4
#define ENTIRE_DISPLAY_OFF 0xA5

#define SET_DISPLAY_OFFSET 0xD3
#define DISPLAY_OFFSET_VAL 0x00

#define SET_FREQUENCY_MODE 0xD5
#define FREQUENCY_VALUE 0xF0 //maybe change later to F0

#define SET_PRECHARGE_PERIOD_MODE 0xD9
#define PRECHARGE_PERIOD_VAL 0x22

#define SET_COM_HARDW_CONF_MODE 0xDA
#define COM_HARD_CONF_MODE 0x12 // 0x12 for 128x64 px 0x02 for 128x32 px

#define SET_VCOM_DESELECT_MODE 0xDB
#define VCOM_DESELECT_VALUE 0x20 //maybe change later to 35

#define SET_DC_CONTROL_MODE 0x8D
#define DC_DC_VAL 0x14

#define SCROLL_OFF 0x2E

typedef enum {BLACK, WHITE} Color;

typedef struct SH1106{
    uint8_t buffer[OLED_W * OLED_H >> 3];
    uint8_t negative, inverted;
    uint8_t font_spacing, font_height, font_width;
    Color color;
}SH1106;

static SH1106 oled;

void oled_write_register(uint8_t data){
    uint8_t src[2] = {CMD_REGISTER, data};

    i2c_write_blocking(I2C_MACRO, SH1106_I2C_ADDR, src, 2, false);
}

void oled_write_register_multi(uint8_t* data, uint16_t count){
    uint8_t src[OLED_W + 1];
    src[0] = DATA_REGISTER;
    for (uint16_t i = 0; i < count; i++){
        src[1 + i] = data[i];
    }
    i2c_write_blocking(I2C_MACRO, SH1106_I2C_ADDR, src, count + 1, false);
}

void oled_update_screen(SH1106* oled){
    for (uint8_t m = 0; m < 8; m++){
        oled_write_register(PAGE_ADDR + m);
        oled_write_register(LOWER_COL_ADDR);
        oled_write_register(HIGHER_COL_ADDR);

        oled_write_register_multi(&oled->buffer[OLED_W*m], OLED_W);
    }
}
/* Reversing black pixel to colored one and colored one to black and vise verse */
void oled_negative_screen(SH1106* oled){
    oled->negative = !oled->negative;
    if (oled->negative)
        oled_write_register(SET_NORMAL_DISPLAY);
    else
        oled_write_register(SET_REVERSE_DISPLAY);
}
/*Inverts screen vertically. Only vertically */
void oled_invert_screen(SH1106* oled){
    oled->inverted = !oled->inverted;
    if (oled->inverted)
        oled_write_register(0xC0);
    else
        oled_write_register(0xC8);
}

void oled_fill(Color color, SH1106* oled){
    memset(oled->buffer, (color == BLACK) ? 0x00 : 0xFF, sizeof(oled->buffer));
}

void oled_set_color(Color color, SH1106* oled){
    oled->color = color;
}

void oled_draw_pixel(uint8_t x, uint8_t y, SH1106* oled){
    x = (oled->inverted) ? 127 - x : x;
    if (x >= OLED_W || y >= OLED_H)
        return;
    uint16_t pos = x + (y >> 3) * OLED_W; // the same as x + (y / 8) * OLED_W

    if (oled->color == WHITE){
        oled->buffer[pos] |= 1 << (y & 7); // y & 7 the same as y % 8
    } else {
        oled->buffer[pos] &= ~(1 << (y & 7));
    }
}

void oled_draw_bitmap(uint8_t x, uint8_t y, uint8_t width, uint8_t height, const uint8_t* bitmap, SH1106* oled){
    uint8_t ch_mask = 0;
    uint16_t size = (width * height + 7) >> 3;

    uint8_t original_x = x;
    uint8_t original_y = y;
   for (uint16_t i = 0; i < size; i++){
       for (uint8_t j = 0; j < 8; j++){
           ch_mask = 0x80 >> j;

           if (bitmap[i] & ch_mask)
               oled_draw_pixel(x, y, oled);
           x++;

           if ((x - original_x) == width){
               y++;
               x = original_x;
           }
           if((y - original_y) == height){
               return;
           }
       }
   }
}

void oled_print_ch(char ch, uint8_t x, uint8_t y, const uint8_t* font, SH1106* oled){
    uint8_t ascii_pos = ch - 32;
    uint8_t font_width = oled->font_width;
    for (uint8_t col = 0; col < font_width; col++){
        for (uint8_t row = 0; row < oled->font_height; row++){
            if (font[ascii_pos*font_width + col] & (1 << row)) {
               oled_draw_pixel(x + col, y + row, oled);
            }
        }
    }
}

void oled_print_str(const char* str, uint8_t x, uint8_t y, const uint8_t* font, SH1106* oled){
    uint8_t spacing = oled->font_spacing;
    while (*str){
        oled_print_ch(*str++, x, y, font, oled);
        x += spacing;
    }
}

/*dig_sep - digits after decimal separation, coma. overal length of float can't exceed 15 digits, otherwise, chage the digit[] array size*/
void oled_print_float(float val, uint8_t dig_sep, uint8_t x, uint8_t y, const uint8_t* font, SH1106* oled){
    char digits[16];
    int int_part = (int)val;
    int float_part = roundf((val - int_part) * powf(10.0f, dig_sep));
    char format[8];
    snprintf(format, 8, "%%d.%%0%dd", dig_sep);
    snprintf(digits, 16, format, int_part, float_part);
    oled_print_str(digits, x, y, font, oled);
}
/*_digits - used for formatting, pass 1 to just print int */
void oled_print_int(int val, uint8_t _digits, uint8_t x, uint8_t y, const uint8_t* font, SH1106* oled){
    char digits[16];
    char format[8];
    snprintf(format, 8, "%%0%dd", _digits);
    sniprintf(digits, 16, format, val);
    oled_print_str(digits, x, y, font, oled);
}

void oled_print_str_formating(const char* str, uint8_t x, uint8_t y, const uint8_t* font, SH1106* oled, ...){
    va_list arg;
    va_start(arg, oled);
    uint8_t spacing = oled->font_spacing;

    const char* ptr = str;
    while (*ptr){
        while (*ptr && *ptr != '%'){
            oled_print_ch(*ptr, x, y, font, oled);
            x += spacing;
            ptr++;
        }
        if (!*ptr) break;
        ptr++;// skipping the % sign
        char format[16];
        //char number[16];
        uint8_t k = 0;
        while (isdigit(*ptr)){
            format[k++] = *ptr++;
        }
        format[k] = '\0';
        //ptr++;
        char tmp[32];
        if (*ptr == 'i' || *ptr == 'd'
            || *ptr == 'u' || *ptr == 'h') {
            int dig = k > 0 ? atoi(format) : 1;
            oled_print_int(va_arg(arg, int), dig, x, y, font, oled);
            x += dig * spacing;
        }
        else if (*ptr == 'f') {
            int dig_sep = k > 0 ? atoi(format) : 2;
            oled_print_float(va_arg(arg, double), dig_sep, x, y, font, oled);
            snprintf(tmp, sizeof(tmp), "%.*f", dig_sep, 0.0);//measure legth of float
            x += (strlen(tmp)+1) * spacing;
        }
        else if (*ptr == 'c') {
            oled_print_ch(va_arg(arg, int), x, y, font, oled);
        }
        else if (*ptr == 's') {
            char* s = va_arg(arg, char*);
            oled_print_str(s, x, y, font, oled);
            x += strlen(s) * spacing;
        } else{
            oled_print_str(format, x, y, font, oled);
        }
        ptr++;
    }
    /*while (*str){
        oled_print_ch(*str++, x, y, font, oled);
        x += spacing;
    }*/
    va_end(arg);
}

void oled_draw_line(uint8_t x1, uint8_t y1, uint8_t x2, uint8_t y2, SH1106* oled){
    //Modified Bresenham’s algorithm
    uint8_t start_x = (x2 < x1) ? x2 : x1;
    uint8_t start_y = (y2 > y1 || x2 > x1) ? y1 : y2;
    uint8_t y_incr = (y2 > y1 || x1 > x2) ? 1 : -1;

    uint8_t dx = (x2 > x1) ? x2-x1 : x1-x2;
    uint8_t dy = (y2 > y1) ? y2-y1 : y1-y2;

    int8_t slop = 2 * (dy);
    int8_t slop_error = slop - (dx);

    for (uint8_t x = start_x, y = start_y; x < dx + start_x; x++){
        slop_error += slop;

        while (slop_error >=0){
            oled_draw_pixel(x, y, oled);
            y += y_incr;
            slop_error -= 2*(dx);
        }
    }

    /*if (y2 >= y1 && x2 > x1){
        for (uint8_t x = start_x, y = y1; x < dx + start_x; x++){
            slop_error += slop;

            while (slop_error >=0){
                oled_draw_pixel(x, y, oled);
                y += 1;
                slop_error -= 2*(dx);
            }
        }
    } else if ( y2 > y1 && x1 > x2) {
        for (uint8_t x = start_x, y = y1; x < dx + start_x; x++){
            slop_error += slop;

            while (slop_error >=0){
                oled_draw_pixel(x, y, oled);
                y += 1;
                slop_error -= 2*(dx);
            }
        }
    }
    else if (x2 > x1) {
        for (uint8_t x = start_x, y = y1; x < dx + start_x; x++){
            slop_error += slop;

            while (slop_error >=0){
                oled_draw_pixel(x, y, oled);
                y -= 1;
                slop_error -= 2*(dx);
            }
        }
    }else if (x1 > x2) {
        for (uint8_t x = start_x, y = y2; x < dx + start_x; x++){
            slop_error += slop;

            while (slop_error >=0){
                oled_draw_pixel(x, y, oled);
                y += 1;
                slop_error -= 2*(dx);
            }
        }
    }*/

}

void oled_draw_hline(uint8_t x, uint8_t y, uint8_t width, SH1106* oled){
    uint8_t end = x + width;
    for(uint8_t cx = x; cx <= end; cx++){
        oled_draw_pixel(cx, y, oled);
    }
}
void oled_draw_vline(uint8_t x, uint8_t y, uint8_t height, SH1106* oled){
    uint8_t end = y + height;
    for(uint8_t cy = y; cy <= end; cy++){
        oled_draw_pixel(x, cy, oled);
    }
}

void oled_draw_rect_filled(uint8_t x, uint8_t y, uint8_t width, uint8_t height, SH1106* oled){
    for (uint8_t ry = y; ry <= y + height; ry ++){
        for (uint8_t rx = x; rx <= x + width; rx++){
            oled_draw_pixel(rx, ry, oled);
        }
    }
}

void oled_draw_rect(uint8_t x, uint8_t y, uint8_t width, uint8_t height, SH1106* oled){
    /*for (uint8_t rx = x; rx < x + width; rx++) oled_draw_pixel(rx, y, oled);
    for (uint8_t rx = x; rx < x + width; rx++) oled_draw_pixel(rx, y + height, oled);
    for (uint8_t ry = y; ry < y + height; ry ++) oled_draw_pixel(x, ry, oled);
    for (uint8_t ry = y; ry < y + height; ry ++) oled_draw_pixel(x + width, ry, oled);*/
    oled_draw_hline(x, y, width, oled);
    oled_draw_hline(x, y + height, width, oled);
    oled_draw_vline(x, y, height, oled);
    oled_draw_vline(x + width, y, height, oled);
}

void oled_set_font3x5(SH1106* oled){
    oled->font_spacing = 4;
    oled->font_height = 5;
    oled->font_width = 3;
}

void oled_set_font5x7(SH1106* oled){
    oled->font_spacing = 6;
    oled->font_height = 7;
    oled->font_width = 5;
}

SH1106* oled_init(void){
    oled_write_register(DISPLAY_OFF);

    //oled_write_register(SET_VCOM_DESELECT_MODE);
    oled_write_register(VCOM_DESELECT_VALUE);
    oled_write_register(0x10);

    oled_write_register(PAGE_ADDR);

    oled_write_register(COM_DIRECTION);

    oled_write_register(LOWER_COL_ADDR);
    oled_write_register(HIGHER_COL_ADDR);

    oled_write_register(START_LINE);

    oled_write_register(SET_CONTRAST_CONTROL_MODE);
    oled_write_register(CONTRAST_VAL);

    oled_write_register(SEGMENT_REMAP);

    oled_write_register(SET_NORMAL_DISPLAY);

    oled_write_register(SET_MULTIPLEX_RATION_MODE);
    oled_write_register(MULTIPLEX_RATION_VAL);

    oled_write_register(ENTIRE_DISPLAY_ON);

    oled_write_register(SET_DISPLAY_OFFSET);
    oled_write_register(DISPLAY_OFFSET_VAL);

    oled_write_register(SET_FREQUENCY_MODE);
    oled_write_register(FREQUENCY_VALUE);

    oled_write_register(SET_PRECHARGE_PERIOD_MODE);
    oled_write_register(PRECHARGE_PERIOD_VAL);

    oled_write_register(SET_COM_HARDW_CONF_MODE);
    oled_write_register(COM_HARD_CONF_MODE);

    oled_write_register(SET_VCOM_DESELECT_MODE);
    oled_write_register(VCOM_DESELECT_VALUE);

    /*oled_write_register(SET_DC_CONTROL_MODE);
        oled_write_register(DC_DC_ON);*/
    oled_write_register(SET_DC_CONTROL_MODE);
    oled_write_register(DC_DC_VAL);

    oled_write_register(SCROLL_OFF);

    oled_write_register(DISPLAY_ON);

    oled.color = WHITE;
    oled.negative = 0;
    oled.inverted = 0;
    oled_set_font5x7(&oled);

    oled_fill(BLACK, &oled);
    oled_update_screen(&oled);

    return &oled;
}

#endif
