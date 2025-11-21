#include "emu.h"

// Constructor initializes components in correct dependency order: Rom -> Bus -> Cpu
Emu::Emu() 
    : rom(), 
      bus(rom), 
      cpu(bus),
      ctx{false, true, 0}
{
}

void Emu::emu_cycles(int cycles)
{
}
