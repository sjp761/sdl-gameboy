# Core gameboy emulation sources
set(GAMEBOY_CORE_SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/bus.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/dma.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/emu.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/lcd.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/ppu.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/timer.cpp
)

# Add subdirectories
add_subdirectory(cpu)
add_subdirectory(rom)

# Set GAMEBOY_SOURCES in parent scope
set(GAMEBOY_SOURCES 
    ${GAMEBOY_SOURCES}
    ${GAMEBOY_CORE_SOURCES}
    PARENT_SCOPE
)

# Set include directories in parent scope
set(GAMEBOY_INCLUDES
    emu/include/gameboy
    emu/include/gameboy/cpu
    emu/include/gameboy/rom
    emu/include/gui
    PARENT_SCOPE
)
