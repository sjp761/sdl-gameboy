cmake_minimum_required(VERSION 3.10.0)
project(sdl-gameboy VERSION 0.1.0 LANGUAGES C CXX)

# Source files
file(GLOB GAMEBOY_SRC "src/gameboy/*.cpp")
file(GLOB GAMEBOY_CMP_SRC "src/gameboy/components/*.cpp")
file(GLOB SDL_SRC "src/sdl/*.cpp")

# Executables
add_executable(sdl-gameboy ${GAMEBOY_SRC} ${GAMEBOY_CMP_SRC} ${SDL_SRC})
add_executable(opcode-test tests/opcodetest.cpp src/gameboy/Configuration.cpp ${GAMEBOY_CMP_SRC} ${SDL_SRC})

# Common include directories
set(GAMEBOY_INCLUDES src/gameboy src/sdl src/gameboy/components)

target_include_directories(sdl-gameboy PRIVATE ${GAMEBOY_INCLUDES})
target_include_directories(opcode-test PRIVATE ${GAMEBOY_INCLUDES})

# SDL dependencies
find_package(SDL3 REQUIRED)
find_package(PkgConfig REQUIRED)
pkg_check_modules(jsoncpp REQUIRED jsoncpp)

foreach(target sdl-gameboy opcode-test)
    target_link_libraries(${target} PRIVATE ${SDL3_LIBRARIES} ${jsoncpp_LIBRARIES})
    target_include_directories(${target} PRIVATE ${jsoncpp_INCLUDE_DIRS})
endforeach()