#include "cpu.h"
#include "emu.h"
#include "bus.h"
#include "instruction.h"
#include <iostream>

void Cpu::cpu_init()
{
}

bool Cpu::cpu_step()
{
    if (!halted)
    {
        fetch_instruction();
        instruction = Instruction(opcode);
        if (instruction.type == IN_NONE)
        {
            std::cerr << "Unknown opcode: " << std::hex << static_cast<int>(opcode.whole) << std::dec << std::endl;
            return false; // Invalid instruction, do not proceed
        }
        fetch_data();
        return true; // Indicate that a step was executed
    }
    return false; // Indicate that no step was executed
}

void Cpu::fetch_data()
{
  
}

void Cpu::emu_cycles()
{
}

void Cpu::fetch_instruction()
{
    opcode = Opcode(Emu::cmp.bus.bus_read(regs.pc++));
    std::cout << "Fetched instruction: " << static_cast<int>(opcode.whole) << std::endl;
}
