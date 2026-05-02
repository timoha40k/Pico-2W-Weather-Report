#ifndef JSON_PARSER_H
#define JSON_PARSER_H

#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include "stdio.h"

const char* json_get_string(const char* json, const char* key, char* out, uint16_t out_size){
    char search[64];
    snprintf(search, sizeof(search), "\"%s\":", key);
    const char* pos = strstr(json, search);
    if (!pos)
        return 0;

    pos += strlen(search);
    while(*pos == ' ') pos++;
    if (*pos != '"') return 0;
    pos++;//skip opening quote

    uint16_t i =0 ;
    while (*pos && *pos != '"' && i < out_size - 1)
        out[i++] = *pos++;
    out[i] = '\0';
    return pos;
}

int json_get_float(const char* json, const char* key, float* out){
    char search[64];
    snprintf(search, sizeof(search), "\"%s\":", key);

    const char *pos = strstr(json, search);
    if (!pos) return 0;

    pos += strlen(search);
    while (*pos == ' ') pos++; // skip whitespace

    return sscanf(pos, "%f", out) == 1;
}
int json_get_int(const char* json, const char* key, int* out) {
    char search[64];
    snprintf(search, sizeof(search), "\"%s\":", key);

    const char *pos = strstr(json, search);
    if (!pos) return 0;

    pos += strlen(search);
    while (*pos == ' ') pos++;

    return sscanf(pos, "%d", out) == 1;
}

int json_get_array_float(const char* json, const char* key, float out[], uint16_t out_size){
    char search[64];
    snprintf(search, sizeof(search), "\"%s\":[", key);
    const char* pos = strstr(json, search);
    if (!pos)
        return 1;
    pos += strlen(search);
    while(*pos == ' ') pos++;

    uint16_t i = 0 ;
    while (*pos && *pos != ']' && i < out_size){
        while (*pos == ' ' || *pos == '\t' || *pos == '\n') pos++;

        char* end;
        out[i++] = strtof(pos, &end);
        pos = end;
        //sscanf(pos, "%f", &out[i]);
        //pos += 2;
        if (*pos == ',') pos++;
    }
    return i;
}

int json_get_array_strings(const char* json, const char* key, char** out, uint16_t out_size, uint16_t out_string_size){
    char search[64];
    snprintf(search, sizeof(search), "\"%s\":[", key);
    const char* pos = strstr(json, search);
    if (!pos)
        return 1;
    pos += strlen(search);
    while(*pos == ' ') pos++;

    uint16_t i = 0;
    while (*pos && *pos != ']' && i < out_size){
        while (*pos == ' ' || *pos == '\t' || *pos == '\n' || *pos == '"' || *pos == ',') pos++;
        if (*pos == ']') break;

        uint16_t j = 0;
        while(*pos != '"' && j < out_string_size - 1 && *pos){
            out[i][j++] = *pos++;
        }
        out[i++][j] = '\0';
    }
    return i;
}

#endif

/*{"latitude":52.23009,"longitude":21.017075,"generationtime_ms":0.1443624496459961,"utc_offset_seconds":7200,"timezone":"Europe/Berlin","timezone_abbreviation":"GMT+2","elevation":113.0,"hourly_units":{"time":"iso8601","temperature_2m":"°C","weather_code":"wmo code"},"hourly":{"time":["2026-04-22T00:00","2026-04-22T01:00","2026-04-22T02:00","2026-04-22T03:00","2026-04-22T04:00","2026-04-22T05:00","2026-04-22T06:00","2026-04-22T07:00","2026-04-22T08:00","2026-04-22T09:00","2026-04-22T10:00","2026-04-22T11:00","2026-04-22T12:00","2026-04-22T13:00","2026-04-22T14:00","2026-04-22T15:00","2026-04-22T16:00","2026-04-22T17:00","2026-04-22T18:00","2026-04-22T19:00","2026-04-22T20:00","2026-04-22T21:00","2026-04-22T22:00","2026-04-22T23:00"],"temperature_2m":[8.3,7.6,7.0,6.7,5.9,4.8,4.3,4.9,6.1,7.9,9.9,11.8,12.9,14.0,14.5,15.0,15.4,15.3,15.1,14.6,13.7,13.7,12.9,12.0],"weather_code":[0,0,0,0,0,1,3,0,0,0,0,1,2,1,3,1,1,0,0,0,3,3,3,3]},"daily_units":{"time":"iso8601","weather_code":"wmo code"},"daily":{"time":["2026-04-22"],"weather_code":[3]}}*/
