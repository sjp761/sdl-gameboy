#include "ppu.h"
#include <cstdint>


Ppu::Ppu() : bus(nullptr)
{
    
}

void Ppu::set_cmp(Bus *bus_ptr)
{
    bus = bus_ptr;
}
