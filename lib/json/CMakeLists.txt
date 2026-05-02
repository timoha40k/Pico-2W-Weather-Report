cmake_minimum_required(VERSION 3.13)
add_library(json_lib INTERFACE)

target_include_directories(json_lib INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})
