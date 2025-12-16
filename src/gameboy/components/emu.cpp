#include "emu.h"
#include "cpu/cpu_tables.h"

// Constructor initializes pointers to nullptr
Emu::Emu() 
    : rom(), 
      bus(),
      cpu(),
      timer(),
      ctx{false, true, 0}
{}

void Emu::set_component_pointers()
{
  cpu.set_cmp(&bus, &timer);
  bus.set_cmp(&rom, &timer);
  timer.set_cmp(&cpu, &bus);
}
