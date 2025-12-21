#pragma once
#include "rom.h"
#include "cpu/cpu.h"
#include "bus.h"
#include <memory>
#include <atomic>
#include "timer.h"
#include "ppu.h"
#include "dma.h"
#include "lcd.h"
struct emu_context 
{
    std::atomic<bool> paused;
    std::atomic<bool> running;
    std::atomic<uint64_t> ticks;
};
    
class Emu
{
    private:
        Rom rom;
        Bus bus;
        Cpu cpu;
        Timer timer;
        Ppu ppu;
        DMA dma;
        LCD lcd;
    public:
        emu_context ctx;
        
        Emu();
        
        // Accessors for components
        Rom& get_rom() { return rom; }
        Bus& get_bus() { return bus; }
        Cpu& get_cpu() { return cpu; }
        Ppu& get_ppu() { return ppu; }
        void set_component_pointers();
};