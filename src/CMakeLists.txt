# Create your executable
add_executable(esp_flasher 
    main.c
    flasher_utils.c
)

# Add port implementation directly to flasher target
target_sources(flasher PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/linux_port.c
)

# Set up include directories
target_include_directories(esp_flasher PRIVATE
    ${CMAKE_SOURCE_DIR}/external/esp-serial-flasher/include
)

target_include_directories(flasher PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_SOURCE_DIR}/external/esp-serial-flasher/include
    
)

# Link ESP Serial Flasher
target_link_libraries(esp_flasher PRIVATE flasher gpiod)