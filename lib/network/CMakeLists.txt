add_library(network_lib INTERFACE)

target_include_directories(network_lib INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})

target_link_libraries(network_lib INTERFACE pico_stdlib
    pico_cyw43_arch_lwip_threadsafe_background
    pico_lwip_http
    pico_lwip_mbedtls
    pico_mbedtls)
