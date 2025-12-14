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

void Emu::emu_cycles()
{
    int opcode = cpu.opcode.whole;
    int cycles = INSTRUCTION_TABLE[opcode].cycles;
    for (int i = 0; i < cycles; ++i) {
        timer.tick();
    }
}

void Emu::set_component_pointers()
{
  cpu.set_cmp(&bus);
  bus.set_cmp(&rom, &timer);
  timer.set_cmp(&cpu, &bus);
}
