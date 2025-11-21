cmake_minimum_required(VERSION 3.10.0)
project(sdl-gameboy VERSION 0.1.0 LANGUAGES C CXX)

## Use C++17 for wide compatibility across compilers and to enable std::filesystem
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS ON)

# Source files
file(GLOB GAMEBOY_SRC "src/gameboy/*.cpp")
file(GLOB GAMEBOY_CMP_SRC "src/gameboy/components/*.cpp")
file(GLOB SDL_SRC "src/sdl/*.cpp")

# Executables
add_executable(sdl-gameboy ${GAMEBOY_SRC} ${GAMEBOY_CMP_SRC} ${SDL_SRC})
add_executable(opcode-test tests/opcodetest.cpp src/gameboy/Configuration.cpp ${GAMEBOY_CMP_SRC} ${SDL_SRC})

# Common include directories
set(GAMEBOY_INCLUDES src/gameboy src/sdl src/gameboy/components)
## Use C++17 for wide compatibility across compilers and to enable std::filesystem
target_include_directories(sdl-gameboy PRIVATE ${GAMEBOY_INCLUDES})
target_include_directories(opcode-test PRIVATE ${GAMEBOY_INCLUDES})

# SDL dependencies
find_package(SDL3 REQUIRED)
find_package(jsoncpp REQUIRED)
target_compile_definitions(opcode-test PRIVATE 
    OPCODETEST=1
)

foreach(target sdl-gameboy opcode-test)
    target_link_libraries(${target} PRIVATE SDL3::SDL3 JsonCpp::JsonCpp)
endforeach()