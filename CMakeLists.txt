cmake_minimum_required(VERSION 3.16)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

project(esp_flasher C)

# Configure ESP Serial Flasher
set(SERIAL_FLASHER_INTERFACE_UART true)
set(PORT USER_DEFINED)

# Add ESP Serial Flasher submodule
add_subdirectory(external/esp-serial-flasher)

add_subdirectory(src)