cmake_minimum_required(VERSION 3.10.0)
project(sdl-gameboy VERSION 0.1.0 LANGUAGES C CXX)

file(GLOB GAMEBOY_SRC "src/gameboy/*")
file(GLOB GAMEBOY_CMP_SRC "src/gameboy/components/*")
file(GLOB SDL_SRC "src/sdl/*")

add_executable(sdl-gameboy ${GAMEBOY_SRC} ${GAMEBOY_CMP_SRC} ${SDL_SRC})

target_include_directories(sdl-gameboy PRIVATE src/gameboy src/sdl)

find_package(SDL3 REQUIRED)
find_package(SDL3_ttf REQUIRED)
include_directories(${SDL3_INCLUDE_DIRS} ${SDL3_ttf_INCLUDE_DIRS})
target_link_libraries(sdl-gameboy ${SDL3_LIBRARIES} ${SDL3_ttf_LIBRARIES})