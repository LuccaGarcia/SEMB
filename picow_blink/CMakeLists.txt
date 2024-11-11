# Generated Cmake Pico project file

cmake_minimum_required(VERSION 3.13)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Initialise pico_sdk from installed location
# (note this can come from environment, CMake cache etc)

# == DO NOT EDIT THE FOLLOWING LINES for the Raspberry Pi Pico VS Code Extension to work ==
if(WIN32)
    set(USERHOME $ENV{USERPROFILE})
else()
    set(USERHOME $ENV{HOME})
endif()
set(sdkVersion 2.0.0)
set(toolchainVersion 13_2_Rel1)
set(picotoolVersion 2.0.0)
set(picoVscode ${USERHOME}/.pico-sdk/cmake/pico-vscode.cmake)
if (EXISTS ${picoVscode})
    include(${picoVscode})
endif()
# ====================================================================================
set(PICO_BOARD pico_w CACHE STRING "Board type")

# Pull in Raspberry Pi Pico SDK (must be before project)
include(pico_sdk_import.cmake)

project(picow_blink C CXX ASM)

# Initialise the Raspberry Pi Pico SDK
pico_sdk_init()

# Add executable. Default name is the project name, version 0.1

add_executable(picow_blink
        picow_blink.c
        )
target_link_libraries(picow_blink
        pico_stdlib              # for core functionality
        pico_cyw43_arch_none     # we need Wifi to access the GPIO, but we don't need anything else
        )

# create map/bin/hex file etc.
pico_add_extra_outputs(picow_blink)

# add url via pico_set_program_url

# This version should behave exactly the same, but it runs the sys clock slowly.
add_executable(picow_blink_slow_clock
        picow_blink_slow_clock.c
        )
target_link_libraries(picow_blink_slow_clock
        pico_stdlib              # for core functionality
        pico_cyw43_arch_none     # we need Wifi to access the GPIO, but we don't need anything else
        hardware_clocks
        )
# This requires us to modify the pio divisor to successfully communicate with the cyw43 chip
target_compile_definitions(picow_blink_slow_clock PRIVATE
        CYW43_PIO_CLOCK_DIV_DYNAMIC=1
        )

# create map/bin/hex file etc.
pico_add_extra_outputs(picow_blink_slow_clock)

# add url via pico_set_program_url
