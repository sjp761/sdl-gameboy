#pragma once
#include "rom.h"
#include "cpu.h"
#include "bus.h"
struct components
{
    Rom rom; // Initialize Rom instance
    Cpu cpu;
    Bus bus;
};

struct emu_context 
{
    bool paused;
    bool running;
    uint64_t ticks;
};
    
class Emu
{
    public:
        static components cmp;
        static emu_context ctx;
};