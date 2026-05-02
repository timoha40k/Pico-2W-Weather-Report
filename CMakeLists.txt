cmake_minimum_required(VERSION 3.13)

include(pico_sdk_import.cmake)

project(main C CXX ASM)

pico_sdk_init()

add_executable(main
    src/main.c
)

add_subdirectory(lib/sh1106)
add_subdirectory(lib/network)
add_subdirectory(lib/json)

target_compile_definitions(main PRIVATE
        WIFI_SSID=\"$ENV{WIFI_SSID}\"
        WIFI_PASSWORD=\"$ENV{WIFI_PASSWORD}\"
        WEATHER_TODAY=\"$ENV{WEATHER_TODAY}\"
        WEATHER_WEEK=\"$ENV{WEATHER_WEEK}\"
        )

target_include_directories(main PRIVATE
    ${CMAKE_CURRENT_LIST_DIR}
     )

target_link_libraries(main
    pico_stdlib
    hardware_i2c
    pico_cyw43_arch_lwip_threadsafe_background
    pico_lwip_http
    pico_lwip_mbedtls
    pico_mbedtls
    oled_lib
    network_lib
    json_lib
)

pico_enable_stdio_usb(main 1)

pico_add_extra_outputs(main)
message(STATUS "WIFI_SSID = $ENV{WIFI_SSID}")

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
