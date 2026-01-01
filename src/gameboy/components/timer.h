#include <cstdint>
#include "cpu.h"
#include "bus.h"
#pragma once

class Timer
{
    uint16_t div, old_div;
    uint8_t tima, tma, tac;
    Bus* bus;
    public:
        Timer();
        // Set component pointers
        void set_cmp(Bus* bus_ptr) { bus = bus_ptr; }
        void tick();
        // MMIO accessors for timer registers
        // 0xFF04 DIV (read upper 8 bits), write resets
        // 0xFF05 TIMA
        // 0xFF06 TMA
        // 0xFF07 TAC
        uint8_t read(uint16_t address);
        void write(uint16_t address, uint8_t value);
};