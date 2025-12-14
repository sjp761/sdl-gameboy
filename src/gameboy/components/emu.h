#pragma once
#include "rom.h"
#include "cpu/cpu.h"
#include "bus.h"
#include <memory>
#include "timer.h"

struct emu_context 
{
    bool paused;
    bool running;
    uint64_t ticks;
};
    
class Emu
{
    private:
        Rom rom;
        Bus bus;
        Cpu cpu;
        Timer timer;
        
    public:
        emu_context ctx;
        
        Emu();
        
        // Accessors for components
        Rom& get_rom() { return rom; }
        Bus& get_bus() { return bus; }
        Cpu& get_cpu() { return cpu; }
        void emu_cycles();
        void set_component_pointers();
        const Rom& get_rom() const { return rom; }
        const Bus& get_bus() const { return bus; }
        const Cpu& get_cpu() const { return cpu; }
};