#include "emu.h"
#include "cpu/cpu_tables.h"

// Constructor initializes pointers to nullptr
Emu::Emu() 
    : rom(), 
      bus(),
      cpu(),
      timer(),
      dma(),
      lcd(),
      ctx{false, true, 0}
{}

void Emu::set_component_pointers()
{
  cpu.set_cmp(&bus, &timer, &dma);
  bus.set_cmp(&rom, &timer, &ppu, &dma, &lcd);
  timer.set_cmp(&cpu, &bus);
  ppu.set_cmp(&bus);
  dma.set_cmp(&bus);
}
