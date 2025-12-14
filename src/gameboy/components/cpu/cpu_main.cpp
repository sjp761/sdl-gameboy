#include "cpu.h"
#include "bus.h"
#include <iostream>
#include <cstdio>
#include <interrupts.h>

Cpu::Cpu()
{
}

void Cpu::cpu_init()
{
    regs.pc = 0x100;
    ime = false; // Interrupts enabled by default
}

bool Cpu::cpu_step()
{
    // Capture delayed IME enable at start of step (EI effect happens after NEXT instruction)
    bool enable_ime_after = ime_delay;

    if (!halted)
    {
        fetch_instruction();
        fetch_data();
        execute_instruction();
        /*
        printf("PC: 0x%04X | Opcode: 0x%02X | A: 0x%02X | F: 0x%02X | BC: 0x%04X | DE: 0x%04X | HL: 0x%04X | SP: 0x%04X\n",
           regs.pc - 1, opcode.whole, regs.a, regs.f,
           (regs.b << 8) | regs.c, (regs.d << 8) | regs.e,
           (regs.h << 8) | regs.l, regs.sp);
        */

    }
    else
    {
        if (bus->if_register)
        {
            halted = false; // Exit halt state if an interrupt is pending
        }
    }
        

    if (ime)
    {
        handle_interrupts();
    }
    // Apply EI delayed enable only if it was pending BEFORE this step started
    if (enable_ime_after) {
        ime = true;
        ime_delay = false;
    }
    read_serial_debug();
    return true;
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

void Cpu::fetch_instruction()
{
    opcode = Opcode(bus->bus_read(regs.pc++));
    
    // Debug output
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

void Cpu::read_serial_debug()
{
    if (bus->bus_read (0xFF02) == 0x81) 
     {
        char c = static_cast<char>(bus->bus_read(0xFF01));
        bus->serial_buffer += c;
        if (c == '\n' || bus->serial_buffer.length() >= 128) 
        {
            std::cout << bus->serial_buffer;
            bus->serial_buffer.clear();
        }
        bus->bus_write(0xFF02, 0x00); // Clear the transfer start flag
     }
}

void Cpu::handle_interrupts()
{   
    // Check interrupts in priority order
    if (check_interrupt(Interrupts::InterruptMask::IT_VBlank)) return;
    if (check_interrupt(Interrupts::InterruptMask::IT_LCDStat)) return;
    if (check_interrupt(Interrupts::InterruptMask::IT_Timer)) return;
    if (check_interrupt(Interrupts::InterruptMask::IT_Serial)) return;
    if (check_interrupt(Interrupts::InterruptMask::IT_Joypad)) return;
}

void Cpu::interrupt_set_pc(uint16_t address)
{
    stack_push16(regs.pc);
    regs.pc = address;
}

bool Cpu::check_interrupt(Interrupts::InterruptMask it)
{
    if ((bus->if_register & static_cast<uint8_t>(it)) && (bus->ie_register & static_cast<uint8_t>(it))) {
        interrupt_set_pc(Interrupts::InterruptPC[it]);
        bus->if_register &= ~static_cast<uint8_t>(it); // Clear the interrupt flag
        ime = false; // Disable further interrupts
        ime_delay = false; // Cancel any pending EI enable when servicing
        halted = false;

        return true;
    }
    return false;
}


//  Stack Operations
void Cpu::stack_push8(uint8_t value) {
    bus->bus_write(--regs.sp, value);
}

void Cpu::stack_push16(uint16_t value) {
    stack_push8(value >> 8);   // Push high byte first
    stack_push8(value & 0xFF); // Push low byte
}