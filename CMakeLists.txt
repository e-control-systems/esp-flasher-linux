cmake_minimum_required(VERSION 3.16)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

project(esp-flasher 
    VERSION 0.2.0
    LANGUAGES C
    DESCRIPTION "Linux utility to program ESP Family of microcontrollers"
)

configure_file(${PROJECT_SOURCE_DIR}/src/config.h.in ${PROJECT_BINARY_DIR}/src/config.h)

# Configure ESP Serial Flasher
set(SERIAL_FLASHER_INTERFACE_UART true)
set(PORT USER_DEFINED)

# Add ESP Serial Flasher submodule
add_subdirectory(external/esp-serial-flasher)

add_subdirectory(src)

option(PACKAGE "Add packaging configurations" ON)
if(PACKAGE)
    message(STATUS "Adding packaing")
    add_subdirectory(packaging)
endif()
