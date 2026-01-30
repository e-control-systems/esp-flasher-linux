# SPDX-FileCopyrightText: 2026 E-Control Systems
#
# SPDX-License-Identifier: Apache-2.0

# Create your executable
add_executable(${PROJECT_NAME} 
    main.c
    flasher_utils.c
    utils.c
)

# Add port implementation directly to flasher target
target_sources(flasher PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/linux_port.c
)

target_include_directories(${PROJECT_NAME} 
    PRIVATE
        ${CMAKE_CURRENT_BINARY_DIR}
)

target_include_directories(${PROJECT_NAME} PRIVATE
    ${CMAKE_SOURCE_DIR}/external/esp-serial-flasher/include
)

target_include_directories(flasher PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_SOURCE_DIR}/external/esp-serial-flasher/include
    
)

# Link ESP Serial Flasher
target_link_libraries(${PROJECT_NAME} PRIVATE flasher gpiod)

install(TARGETS ${PROJECT_NAME} DESTINATION bin)