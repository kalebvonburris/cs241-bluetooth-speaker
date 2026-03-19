cmake_minimum_required(VERSION 3.13)

include($ENV{PICO_SDK_PATH}/external/pico_sdk_import.cmake)

project(pico_blink C CXX ASM)
set(CMAKE_C_STANDARD   11)
set(CMAKE_CXX_STANDARD 17)

pico_sdk_init()

add_executable(pico_blink main.cpp)

target_link_libraries(pico_blink
    pico_stdlib
    pico_cyw43_arch_none
)

pico_enable_stdio_uart(pico_blink 1)
pico_enable_stdio_usb(pico_blink  0)

pico_add_extra_outputs(pico_blink)

add_custom_target(flash
    COMMAND picotool load -f ${CMAKE_BINARY_DIR}/pico_blink.uf2 -x
    DEPENDS pico_blink
    COMMENT "Flashing via picotool"
)
