#include "cpu.h"
#include "emu.h"
#include "bus.h"
#include <iostream>

void Cpu::cpu_init()
{
    regs.pc = 0x100;
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
   // LD r16, u16 instructions (x=0, z=1) require 16-bit immediate
   if (opcode.x == 0 && opcode.z == 1)
   {
    // Little-endian: low byte first, then high byte
    uint8_t low = Emu::cmp.bus.bus_read(regs.pc);
    uint8_t high = Emu::cmp.bus.bus_read(regs.pc + 1);
    fetched_data = (high << 8) | low;
    regs.pc += 2;
   }
   // LD r8, u8 instructions (x=0, z=6) require 8-bit immediate
   else if (opcode.x == 0 && opcode.z == 6)
   {
    fetched_data = Emu::cmp.bus.bus_read(regs.pc++);
   }
}

void Cpu::emu_cycles()
{
}

void Cpu::fetch_instruction()
{
    opcode = Opcode(Emu::cmp.bus.bus_read(regs.pc++));
}

void Cpu::execute_instruction()
{
    // LD r16, u16: x=0, z=1, y determines register pair
    if (opcode.x == 0 && opcode.z == 1)
    {
        switch (opcode.y >> 1) { // y/2 gives us 0,1,2,3 for BC,DE,HL,SP
            case 0: // LD BC, u16 (y=0)
                regs.c = fetched_data & 0xFF;
                regs.b = (fetched_data >> 8) & 0xFF;
                break;
            case 1: // LD DE, u16 (y=2)
                regs.e = fetched_data & 0xFF;
                regs.d = (fetched_data >> 8) & 0xFF;
                break;
            case 2: // LD HL, u16 (y=4)
                regs.l = fetched_data & 0xFF;
                regs.h = (fetched_data >> 8) & 0xFF;
                break;
            case 3: // LD SP, u16 (y=6)
                regs.sp = fetched_data;
                break;
        }
    }
    // LD r8, u8: x=0, z=6, y determines register
    else if (opcode.x == 0 && opcode.z == 6)
    {
        uint8_t value = fetched_data & 0xFF;
        switch (opcode.y) {
            case 0: // LD B, u8 (0x06)
                regs.b = value;
                break;
            case 1: // LD C, u8 (0x0E)
                regs.c = value;
                break;
            case 2: // LD D, u8 (0x16)
                regs.d = value;
                break;
            case 3: // LD E, u8 (0x1E)
                regs.e = value;
                break;
            case 4: // LD H, u8 (0x26)
                regs.h = value;
                break;
            case 5: // LD L, u8 (0x2E)
                regs.l = value;
                break;
            case 6: // LD (HL), u8 (0x36) - write to memory at HL
                {
                    uint16_t addr = (regs.h << 8) | regs.l;
                    Emu::cmp.bus.bus_write(addr, value);
                }
                break;
            case 7: // LD A, u8 (0x3E)
                regs.a = value;
                break;
        }
    }
}
