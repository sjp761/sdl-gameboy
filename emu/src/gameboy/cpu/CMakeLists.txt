set(CPU_SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/cpu_alu.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/cpu_helpers.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/cpu_instructions_cb.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/cpu_instructions_x0.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/cpu_instructions_x1.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/cpu_instructions_x2.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/cpu_instructions_x3.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/cpu_main.cpp
)

set(GAMEBOY_SOURCES 
    ${GAMEBOY_SOURCES}
    ${CPU_SOURCES}
    PARENT_SCOPE
)
