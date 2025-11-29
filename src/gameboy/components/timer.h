#include <cstdint>
#include "cpu.h"
#include "bus.h"
#pragma once

class Timer
{
    uint16_t div;
    uint8_t tima, tma, tac;
    Cpu& cpu;
    Bus& bus;
    public:
        Timer(Cpu& cpu_ref, Bus& bus_ref);
        void tick();
        // MMIO accessors for timer registers
        // 0xFF04 DIV (read upper 8 bits), write resets
        // 0xFF05 TIMA
        // 0xFF06 TMA
        // 0xFF07 TAC
        uint8_t read(uint16_t address);
        void write(uint16_t address, uint8_t value);
};