add_library(oled_lib INTERFACE)

target_include_directories(oled_lib INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})

target_link_libraries(oled_lib INTERFACE pico_stdlib hardware_i2c)
