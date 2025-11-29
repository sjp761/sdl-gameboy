#include "emu.h"
#include "cpu/cpu_tables.h"

// Constructor initializes components in correct dependency order: Rom -> Bus -> Cpu
Emu::Emu() 
    : rom(), 
      bus(rom), 
      cpu(bus),
      timer(cpu, bus),
      ctx{false, true, 0}
{
  // Link bus to timer for MMIO/register interactions
  bus.attach_timer(timer);
}

void Emu::emu_cycles()
{
    int opcode = cpu.opcode.whole;
    int cycles = INSTRUCTION_TABLE[opcode].cycles;
    for (int i = 0; i < cycles; ++i) {
        timer.tick();
    }
}
