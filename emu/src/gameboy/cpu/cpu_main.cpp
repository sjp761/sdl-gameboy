#include "cpu.h"
#include "bus.h"
#include "timer.h"
#include "dma.h"
#include "ppu.h"
#include <iostream>
#include <cstdio>
#include <interrupts.h>
#include <thread>
#include <chrono>

Cpu::Cpu() : bus(nullptr), timer(nullptr), dma(nullptr), ppu(nullptr),
             fetched_data(0), mem_dest(0), opcode(), halted(false), 
             stepping(false), ime(false), ime_delay(false), branch_taken(false)
{
    // Initialize all CPU registers to zero
    regs.a = 0;
    regs.f = 0;
    regs.b = 0;
    regs.c = 0;
    regs.d = 0;
    regs.e = 0;
    regs.h = 0;
    regs.l = 0;
    regs.pc = 0;
    regs.sp = 0;
}

void Cpu::cpu_init()
{
    regs.pc = 0x0000;  // Start at 0x0000 for bootrom (bootrom will jump to 0x0100 when complete)
    ime = false; // Interrupts disabled by default
}

bool Cpu::cpu_step()
{
    // Capture delayed IME enable at start of step (EI effect happens after NEXT instruction)
    bool enable_ime_after = ime_delay;

    // Print CPU state and next 4 bytes at PC before instruction
    uint16_t pc = regs.pc;
    /*printf(
        "A: %02X F: %02X B: %02X C: %02X D: %02X E: %02X H: %02X L: %02X SP: %04X PC: %02X:%04X (%02X %02X %02X %02X)\n",
        regs.a, regs.f, regs.b, regs.c, regs.d, regs.e, regs.h, regs.l, regs.sp, (pc >> 8) & 0xFF, pc,
        bus->bus_read(pc), bus->bus_read(pc+1), bus->bus_read(pc+2), bus->bus_read(pc+3)
    );*/

    if (!halted)
    {
        fetch_instruction();
        fetch_data();
        execute_instruction();
        if (!branch_taken)
            emu_cycles(INSTRUCTION_TABLE[opcode.whole].cycles);
        else
        {
            emu_cycles(INSTRUCTION_TABLE[opcode.whole].cycles_branch);
            branch_taken = false; // Reset for next instruction
        }
    }
    else
    {
        // During HALT, CPU consumes 4 T-cycles per iteration
        emu_cycles(1);
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
    if (bus->bus_read(0xFF02) == 0x81) 
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

void Cpu::emu_cycles(int m_cycles)
{
    for (int i = 0; i < m_cycles * 4; ++i) 
    {
        timer->tick();
        ppu->ppu_tick();
    }
    for (int i = 0; i < m_cycles; ++i) {
        if (dma->is_active()) {
            dma->tick();
        }
        //std::this_thread::sleep_for(std::chrono::microseconds(1)); // Simulate timing delay, arbitrary value for now
    }
    
}

void Cpu::request_interrupt(Interrupts::InterruptMask it)
{
    bus->if_register |= static_cast<uint8_t>(it); 
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