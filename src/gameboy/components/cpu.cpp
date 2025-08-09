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
        fetch_data();
        execute_instruction();
        return true; // Indicate that a step was executed
    }
    return false; // Indicate that no step was executed
}

void Cpu::fetch_data()
{
    mem_dest = 0;
    switch (current_instruction->desc)
    {
        case AM_IMP:
            // No operand, nothing to fetch
            break;
        case AM_R_N16:
            // Fetch 16-bit immediate value
            fetched_data = (Emu::cmp.bus.bus_read(regs.pc++) << 8) | Emu::cmp.bus.bus_read(regs.pc++);
            break;
        case AM_R_N8:
            // Fetch 8-bit immediate value
            fetched_data = Emu::cmp.bus.bus_read(regs.pc++);
            break;
        case AM_R_R:
            // No operand, just registers involved
            fetched_data = 0;
            break;
        case AM_MR_R:
            // Memory at register, fetch the value from memory
            fetched_data = Emu::cmp.bus.bus_read((regs.h << 8) | regs.l);
            break;
    }
}

void Cpu::emu_cycles()
{
}

void Cpu::fetch_instruction()
{
    opcode = Emu::cmp.bus.bus_read(regs.pc++);
    current_instruction = Instruction::instruction_by_opcode(opcode);
    if (current_instruction == nullptr) 
    {
        std::cerr << "Unknown opcode: " << std::hex << static_cast<int>(opcode) << std::dec << std::endl;
        exit(1); // Handle unknown opcode
    }
}

void Cpu::execute_instruction()
{
    
}

uint16_t CPUutils::read_register(reg_type rt)
{
    switch (rt)
    {
        case RT_A: return Emu::cmp.cpu.regs.a;
        case RT_F: return Emu::cmp.cpu.regs.f;
        case RT_B: return Emu::cmp.cpu.regs.b;
        case RT_C: return Emu::cmp.cpu.regs.c;
        case RT_D: return Emu::cmp.cpu.regs.d;
        case RT_E: return Emu::cmp.cpu.regs.e;
        case RT_H: return Emu::cmp.cpu.regs.h;
        case RT_L: return Emu::cmp.cpu.regs.l;
        case RT_AF: return (Emu::cmp.cpu.regs.a << 8) | Emu::cmp.cpu.regs.f;
        case RT_BC: return (Emu::cmp.cpu.regs.b << 8) | Emu::cmp.cpu.regs.c;
        case RT_DE: return (Emu::cmp.cpu.regs.d << 8) | Emu::cmp.cpu.regs.e;
        case RT_HL: return (Emu::cmp.cpu.regs.h << 8) | Emu::cmp.cpu.regs.l;
        case RT_SP: return Emu::cmp.cpu.regs.sp;
        case RT_PC: return Emu::cmp.cpu.regs.pc;
        default:
            std::cerr << "Invalid register type." << std::endl;
            exit(1);
    }
}
