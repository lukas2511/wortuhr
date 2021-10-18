cmake_minimum_required(VERSION 3.12)

# Pull in SDK (must be before project)
include($ENV{PICO_SDK_PATH}/external/pico_sdk_import.cmake)

project(blink C CXX ASM)
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

# Initialize the SDK
pico_sdk_init()

# App specific
add_executable(${PROJECT_NAME}
  main.c
)

# create map/bin/hex file etc.
pico_add_extra_outputs(${PROJECT_NAME})

# Pull in our pico_stdlib which pulls in commonly used features
target_link_libraries(${PROJECT_NAME}
  pico_stdlib hardware_i2c
)

# Enable usb output, disable uart output
pico_enable_stdio_usb(${PROJECT_NAME} 1)
pico_enable_stdio_uart(${PROJECT_NAME} 0)
