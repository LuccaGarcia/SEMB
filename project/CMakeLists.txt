# Generated Cmake Pico project file
cmake_minimum_required(VERSION 3.13)

include(pico_sdk_import.cmake)
include(pico_extras_import.cmake)
include(FreeRTOS_Kernel_import.cmake)

project(main C CXX ASM)
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(PICO_BOARD pico_w CACHE STRING "Board type")

# Initialise the Raspberry Pi Pico SDK
pico_sdk_init()

# Add executable. 
add_executable(main src/main.c src/game.c src/vga.c)

# Pico SDK Libraries
target_link_libraries( main
    pico_stdlib
    pico_multicore
    pico_scanvideo_dpi
)

# FreeRTOS Libraries
target_link_libraries( main
    FreeRTOS-Kernel
    FreeRTOS-Kernel-Heap1
)

# Add the standard include files to the build
target_include_directories( main PRIVATE
    ${CMAKE_CURRENT_LIST_DIR}
)

target_compile_options( main PUBLIC
    ### Gnu/Clang C Options
    $<$<COMPILE_LANG_AND_ID:C,GNU>:-fdiagnostics-color=always>
    $<$<COMPILE_LANG_AND_ID:C,Clang>:-fcolor-diagnostics>

    $<$<COMPILE_LANG_AND_ID:C,Clang,GNU>:-Wall>
    $<$<COMPILE_LANG_AND_ID:C,Clang,GNU>:-Wextra>
    # $<$<COMPILE_LANG_AND_ID:C,Clang,GNU>:-Werror>
    $<$<COMPILE_LANG_AND_ID:C,Clang>:-Weverything>
)

pico_set_program_name(main "pico_vga_arcade")
pico_set_program_version(main "0.1")

pico_enable_stdio_uart(main 0)
pico_enable_stdio_usb(main 1)

pico_add_extra_outputs(main)

