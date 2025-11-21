#include "cpu.h"
#include "bus.h"
#include <iostream>

Cpu::Cpu(Bus& bus_ref) : bus(bus_ref), fetched_data(0), mem_dest(0), halted(false), stepping(false), ime(true) {}

void Cpu::cpu_init()
{
    regs.pc = 0x100;
    ime = true; // Interrupts enabled by default
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
   // Decode opcode directly to determine if we need to fetch immediate data
   
   // x=0 instructions
   if (opcode.x == 0) {
       if (opcode.z == 1 && !(opcode.y & 1)) {
           // LD r16, u16 (even y values)
           fetched_data = read_imm16();
       } else if (opcode.z == 6) {
           // LD r8, u8
           fetched_data = read_imm8();
       }
   }
   // x=3 instructions
   else if (opcode.x == 3) {
       if (opcode.z == 2 && opcode.y < 4) {
           // JP cc, u16
           fetched_data = read_imm16();
       } else if (opcode.z == 3 && opcode.y == 0) {
           // JP u16
           fetched_data = read_imm16();
       } else if (opcode.z == 4 && opcode.y < 4) {
           // CALL cc, u16
           fetched_data = read_imm16();
       } else if (opcode.z == 5 && opcode.y == 1) {
           // CALL u16
           fetched_data = read_imm16();
       } else if (opcode.z == 6) {
           // ALU A, u8
           fetched_data = read_imm8();
       }
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
