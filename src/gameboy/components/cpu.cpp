#include "cpu.h"
#include "emu.h"
#include "bus.h"
#include <iostream>

// Fast register lookup - avoids switch statements
static inline uint8_t* get_reg_ptr(cpu_registers& regs, int index) {
    uint8_t* table[8] = {
        &regs.b, &regs.c, &regs.d, &regs.e,
        &regs.h, &regs.l, nullptr, &regs.a
    };
    return table[index];
}

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
   // JP u16 (x=3, z=3, y=0): unconditional jump requires 16-bit address
   else if (opcode.x == 3 && opcode.z == 3 && opcode.y == 0)
   {
    uint8_t low = Emu::cmp.bus.bus_read(regs.pc);
    uint8_t high = Emu::cmp.bus.bus_read(regs.pc + 1);
    fetched_data = (high << 8) | low;
    regs.pc += 2;
   }
   // ALU A, u8 (x=3, z=6): ALU operations with immediate require 8-bit data
   else if (opcode.x == 3 && opcode.z == 6)
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
    switch (opcode.x)
    {
        case 0: // x=0: Various load and arithmetic operations
            switch (opcode.z)
            {
                case 1: // LD r16, u16: y determines register pair
                {
                    // Use pointer arithmetic to avoid repeated shifts
                    uint8_t low = static_cast<uint8_t>(fetched_data);
                    uint8_t high = static_cast<uint8_t>(fetched_data >> 8);
                    
                    switch (opcode.y >> 1) { // y/2 gives us 0,1,2,3 for BC,DE,HL,SP
                        case 0: // LD BC, u16 (y=0)
                            regs.c = low;
                            regs.b = high;
                            break;
                        case 1: // LD DE, u16 (y=2)
                            regs.e = low;
                            regs.d = high;
                            break;
                        case 2: // LD HL, u16 (y=4)
                            regs.l = low;
                            regs.h = high;
                            break;
                        case 3: // LD SP, u16 (y=6)
                            regs.sp = fetched_data;
                            break;
                    }
                    break;
                }

                case 5: // DEC r8: y determines register
                {
                    if (opcode.y == 6) { // DEC (HL) (0x35) - special case
                        uint16_t hl = (static_cast<uint16_t>(regs.h) << 8) | regs.l;
                        uint8_t value = Emu::cmp.bus.bus_read(hl) - 1;
                        Emu::cmp.bus.bus_write(hl, value);
                        // Set flags: Z is set if result is 0, N is set, H is set if borrow from bit 4
                        regs.f = (value == 0 ? 0x80 : 0) | 0x40 | ((value & 0x0F) == 0x0F ? 0x20 : 0) | (regs.f & 0x10);
                        return;
                    }
                    
                    // Direct register access using array indexing
                    uint8_t* reg_ptr = get_reg_ptr(regs, opcode.y);
                    (*reg_ptr)--;
                    uint8_t value = *reg_ptr;
                    // Set flags: Z if result is 0, N=1, H if borrow from bit 4, C unchanged
                    regs.f = (value == 0 ? 0x80 : 0) | 0x40 | ((value & 0x0F) == 0x0F ? 0x20 : 0) | (regs.f & 0x10);
                    break;
                }

                case 6: // LD r8, u8: y determines register
                {
                    uint8_t value = static_cast<uint8_t>(fetched_data);
                    
                    if (opcode.y == 6) { // LD (HL), u8 (0x36) - special case
                        uint16_t hl = (static_cast<uint16_t>(regs.h) << 8) | regs.l;
                        Emu::cmp.bus.bus_write(hl, value);
                    } else {
                        // Direct register access
                        *get_reg_ptr(regs, opcode.y) = value;
                    }
                    break;
                }
            }
            break;

        case 2: // ALU A, r8: y determines operation, z determines source register
        {
            // Get operand - special case for (HL) and direct register access for others
            uint8_t operand;
            if (opcode.z == 6) {
                uint16_t hl = (static_cast<uint16_t>(regs.h) << 8) | regs.l;
                operand = Emu::cmp.bus.bus_read(hl);
            } else {
                operand = *get_reg_ptr(regs, opcode.z);
            }
            
            switch (opcode.y) {
                case 5: // XOR (0xAF when z=7)
                    regs.a ^= operand;
                    regs.f = (regs.a == 0 ? 0x80 : 0); // Z flag if result is 0, all other flags cleared
                    break;
                // Add other ALU operations here as needed
            }
            break;
        }

        case 3: // x=3: Jump instructions and interrupt control
            switch (opcode.z)
            {
                case 3: // JP and interrupt control instructions
                    switch (opcode.y)
                    {
                        case 0: // JP u16 (unconditional jump)
                            regs.pc = fetched_data; // Jump to the address
                            break;
                        case 6: // DI (0xF3) - Disable Interrupts
                            ime = false;
                            break;
                        case 7: // EI (0xFB) - Enable Interrupts
                            ime = true;
                            break;
                    }
                    break;
            }
            break;
    }
}
