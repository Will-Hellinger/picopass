cmake_minimum_required(VERSION 3.13)
include(pico_sdk_import.cmake)

project(picopass C CXX ASM)
pico_sdk_init()

add_executable(picopass
    main.c
    picopass/picopass_bonded.c
    picopass/picopass_adv.c
    picopass/picopass_gap.c
    picopass/picopass_led.c
    picopass/picopass_log.c
    picopass/picopass_secret.c
)

target_include_directories(picopass PRIVATE
    ${CMAKE_CURRENT_LIST_DIR}
)

target_compile_definitions(picopass PRIVATE
    PICO_CYW43_ARCH_NONE=1
)

target_link_libraries(picopass
    pico_stdlib
    pico_rand
    pico_btstack_ble
    pico_btstack_cyw43
    pico_cyw43_arch_none
)

pico_enable_stdio_usb(picopass 1)
pico_enable_stdio_uart(picopass 0)

pico_add_extra_outputs(picopass)

target_compile_options(picopass PRIVATE -Os)
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -s")