cmake_minimum_required(VERSION 3.16)

project(sdl-gameboy VERSION 0.1.0 LANGUAGES C CXX)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

# Use C++20 for modern features
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS ON)

# Find dependencies
find_package(Qt6 COMPONENTS Widgets REQUIRED)

find_package(SDL3 REQUIRED)
find_package(SDL3_ttf REQUIRED)
find_package(jsoncpp REQUIRED)

# Include directories
include_directories(${CMAKE_CURRENT_SOURCE_DIR})

# Source files
file(GLOB GAMEBOY_SRC "emu/src/gameboy/*.cpp")
file(GLOB GAMEBOY_CMP_SRC "emu/src/gameboy/*.cpp")
file(GLOB CPU_SRC "emu/src/gameboy/cpu/*.cpp")

set(GAMEBOY_INCLUDES emu/include/gameboy emu/include/gui emu/include/gameboy/ emu/include/gameboy/cpu)

# Common SDL-Qt sources (excluding debug viewers)
set(SDL_QT_COMMON_SOURCES
    emu/src/gui/main.cpp
    emu/src/gui/mainwindow.cpp
    emu/include/gui/mainwindow.h
    emu/src/gui/mainwindow.ui
    emu/src/gui/SDLContainer.cpp
    emu/include/gui/SDLContainer.h
    emu/src/gui/SDLWidget.cpp
    emu/include/gui/SDLWidget.h
    emu/include/gui/SDL_SmartPointer.h
)

# Debug viewer sources
set(DEBUG_VIEWER_SOURCES
    emu/src/gui/SDL_TileViewer.cpp
    emu/include/gui/SDL_TileViewer.h
    emu/src/gui/SDL_TileMapViewer.cpp
    emu/include/gui/SDL_TileMapViewer.h
)

# Function to create Qt executable
function(create_qt_executable TARGET_NAME SOURCES)
    qt_add_executable(${TARGET_NAME}
        MANUAL_FINALIZATION
        ${SOURCES}
    )

    target_sources(${TARGET_NAME} PRIVATE
        ${GAMEBOY_SRC}
        ${GAMEBOY_CMP_SRC}
        ${CPU_SRC}
    )

    target_link_libraries(${TARGET_NAME} PRIVATE
        Qt6::Widgets
        SDL3::SDL3
        SDL3_ttf::SDL3_ttf
    )

    target_include_directories(${TARGET_NAME} PRIVATE
        ${GAMEBOY_INCLUDES}
    )

    set_target_properties(${TARGET_NAME} PROPERTIES
        MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
        MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
        MACOSX_BUNDLE TRUE
        WIN32_EXECUTABLE TRUE
    )

    qt_finalize_executable(${TARGET_NAME})

    include(GNUInstallDirs)
    install(TARGETS ${TARGET_NAME}
        BUNDLE DESTINATION .
        LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    )
endfunction()

# Regular SDL-Qt GUI executable
if(Qt6_FOUND)
    create_qt_executable(sdl-gameboy-qt "${SDL_QT_COMMON_SOURCES}")
endif()

# Debug viewers executable
if(Qt6_FOUND)
    set(DEBUG_SOURCES ${SDL_QT_COMMON_SOURCES} ${DEBUG_VIEWER_SOURCES})
    create_qt_executable(sdl-gameboy-qt-debug-viewers "${DEBUG_SOURCES}")
    target_compile_definitions(sdl-gameboy-qt-debug-viewers PRIVATE ENABLE_DEBUG_VIEWERS)
endif()

# Opcode test executable
add_executable(opcode-test
    tests/opcodetest.cpp
    tests/test_helper.cpp
)

target_sources(opcode-test PRIVATE
    ${GAMEBOY_SRC}
    ${GAMEBOY_CMP_SRC}
    ${CPU_SRC}
)

target_link_libraries(opcode-test PRIVATE JsonCpp::JsonCpp)

target_include_directories(opcode-test PRIVATE ${GAMEBOY_INCLUDES})
