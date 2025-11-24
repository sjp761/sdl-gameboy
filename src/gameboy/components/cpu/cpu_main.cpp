#include "cpu.h"
#include "bus.h"
#include <iostream>

Cpu::Cpu(Bus& bus_ref) : bus(bus_ref), fetched_data(0), mem_dest(0), halted(false), stepping(false), ime(true) {}

void Cpu::cpu_init()
{
    regs.pc = 0x100;
    ime = false; // Interrupts enabled by default
}

bool Cpu::cpu_step()
{
    if (!halted)
    {
        // Check if IME should be enabled from previous EI instruction
        bool enable_ime_after = ime_delay;
        
        fetch_instruction();
        fetch_data();
        execute_instruction();
        
        // Apply IME change after instruction execution (from previous EI)
        if (enable_ime_after) {
            ime = true;
            ime_delay = false;
        }
        
        return true; // Indicate that a step was executed
    }
    return false; // Indicate that no step was executed
}

void Cpu::fetch_data()
{
    InstructionInfo info = INSTRUCTION_TABLE[opcode.whole];
    switch (info.imm_size) 
    {
        case 1:
            fetched_data = read_imm8();
            break;
        case 2:
            fetched_data = read_imm16();
            break;
        default:
            fetched_data = 0;
            break;
    }
}

void Cpu::emu_cycles()
{
}

void Cpu::fetch_instruction()
{
    opcode = Opcode(bus.bus_read(regs.pc++));
}

void Cpu::execute_instruction()
{
    // CB-prefixed instructions
    if (opcode.whole == 0xCB) {
        execute_cb_instructions();
        return;
    }
    
    // Dispatch based on x field
    switch (opcode.x) {
        case 0:
            execute_x0_instructions();
            break;
        case 1:
            execute_x1_instructions();
            break;
        case 2:
            execute_x2_instructions();
            break;
        case 3:
            execute_x3_instructions();
            break;
    }
}
