# Generated Cmake Pico project file

cmake_minimum_required(VERSION 3.13)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Initialise pico_sdk from installed location
# (note this can come from environment, CMake cache etc)

set(CMAKE_BUILD_TYPE "Debug" CACHE STRING
"Choose the type of Debug, options are: Build Debug Release RelWithDebInfo MinSizeRel."
FORCE)

# == DO NOT EDIT THE FOLLOWING LINES for the Raspberry Pi Pico VS Code Extension to work ==
if(WIN32)
    set(USERHOME $ENV{USERPROFILE})
else()
    set(USERHOME $ENV{HOME})
endif()
set(sdkVersion 2.1.0)
set(toolchainVersion 13_3_Rel1)
set(picotoolVersion 2.1.0)
set(picoVscode ${USERHOME}/.pico-sdk/cmake/pico-vscode.cmake)
if (EXISTS ${picoVscode})
    include(${picoVscode})
endif()
# ====================================================================================
set(PICO_BOARD pico2_w CACHE STRING "Board type")

# Pull in Raspberry Pi Pico SDK (must be before project)
include(pico_sdk_import.cmake)

project(NandFlashTest C CXX ASM)

# Initialise the Raspberry Pi Pico SDK
pico_sdk_init()

# Add executable. Default name is the project name, version 0.1

add_executable(NandFlashTest
NandFlashTest.c
norflash.c
)

pico_set_program_name(NandFlashTest "NandFlashTest")
pico_set_program_version(NandFlashTest "0.1")

# Modify the below lines to enable/disable output over UART/USB
pico_enable_stdio_uart(NandFlashTest 1)
pico_enable_stdio_usb(NandFlashTest 0)

# Add the standard library to the build
target_link_libraries(NandFlashTest
        pico_stdlib)

# Add the standard include files to the build
target_include_directories(NandFlashTest PRIVATE
        ${CMAKE_CURRENT_LIST_DIR}
)

# Add any user requested libraries
target_link_libraries(NandFlashTest 
        hardware_spi
        hardware_dma
        
        )

pico_add_extra_outputs(NandFlashTest)

