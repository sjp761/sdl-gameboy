cmake_minimum_required(VERSION 3.16)

project(sdl-gameboy VERSION 0.1.0 LANGUAGES C CXX)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

## Use C++17 for wide compatibility across compilers and to enable std::filesystem
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS ON)

# Find Qt packages (optional for SDL-Qt integration)
find_package(QT NAMES Qt6 Qt5 COMPONENTS Widgets)
if(QT_FOUND)
    find_package(Qt${QT_VERSION_MAJOR} COMPONENTS Widgets)
endif()

# Find SDL3 and other dependencies
find_package(SDL3 REQUIRED)
find_package(jsoncpp REQUIRED)

include_directories(${CMAKE_CURRENT_SOURCE_DIR})

# Source files for gameboy components
file(GLOB GAMEBOY_SRC "src/gameboy/*.cpp")
file(GLOB GAMEBOY_CMP_SRC "src/gameboy/components/*.cpp")
file(GLOB CPU_SRC "src/gameboy/components/cpu/*.cpp")
file(GLOB SDL_SRC "src/sdl/*.cpp")

# Common include directories
set(GAMEBOY_INCLUDES src/gameboy src/sdl src/gameboy/components src/gameboy/components/cpu)

# SDL-Qt GUI executable (if Qt is available)
if(QT_FOUND)
    set(SDL_QT_SOURCES
            src/sdl/main.cpp
            src/sdl/mainwindow.cpp
            src/sdl/mainwindow.h
            src/sdl/mainwindow.ui
            src/sdl/SDLContainer.cpp
            src/sdl/SDLContainer.h
            src/sdl/SDLWidget.cpp
            src/sdl/SDLWidget.h
            src/sdl/SDL_SmartPointer.h
            src/sdl/SDL_TileViewer.cpp
            src/sdl/SDL_TileViewer.h
    )
endif()

# SDL-Qt GUI target (optional, if Qt is available)
if(QT_FOUND)
    if(${QT_VERSION_MAJOR} GREATER_EQUAL 6)
        qt_add_executable(sdl-gameboy-qt
            MANUAL_FINALIZATION
            ${SDL_QT_SOURCES}
        )
    else()
        if(ANDROID)
            add_library(sdl-gameboy-qt SHARED ${SDL_QT_SOURCES})
        else()
            add_executable(sdl-gameboy-qt ${SDL_QT_SOURCES})
        endif()
    endif()

    # Add emulator sources so symbols resolve when starting Emu from the Qt app
    target_sources(sdl-gameboy-qt PRIVATE
        ${GAMEBOY_SRC}
        ${GAMEBOY_CMP_SRC}
        ${CPU_SRC}
    )

    target_link_libraries(sdl-gameboy-qt PRIVATE Qt${QT_VERSION_MAJOR}::Widgets SDL3::SDL3 JsonCpp::JsonCpp)
    target_include_directories(sdl-gameboy-qt PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src/sdl ${GAMEBOY_INCLUDES})
    
    # Qt for iOS sets MACOSX_BUNDLE_GUI_IDENTIFIER automatically since Qt 6.1.
    if(${QT_VERSION} VERSION_LESS 6.1.0)
        set(BUNDLE_ID_OPTION MACOSX_BUNDLE_GUI_IDENTIFIER com.example.sdl-gameboy)
    endif()
    
    set_target_properties(sdl-gameboy-qt PROPERTIES
        ${BUNDLE_ID_OPTION}
        MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
        MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
        MACOSX_BUNDLE TRUE
        WIN32_EXECUTABLE TRUE
    )

    if(QT_VERSION_MAJOR EQUAL 6)
        qt_finalize_executable(sdl-gameboy-qt)
    endif()

    include(GNUInstallDirs)
    install(TARGETS sdl-gameboy-qt
        BUNDLE DESTINATION .
        LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    )
endif()
