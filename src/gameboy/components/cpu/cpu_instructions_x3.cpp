#include "cpu.h"
#include "bus.h"

//  Stack Operations
uint8_t Cpu::stack_pop8() {
    return bus->bus_read(regs.sp++);
}

uint16_t Cpu::stack_pop16() {
    uint8_t low = stack_pop8();   // Pop low byte first
    uint8_t high = stack_pop8();  // Pop high byte
    return (static_cast<uint16_t>(high) << 8) | low;
}

//  Specific Instruction Handlers for x3 group 
void Cpu::handle_alu_imm8() {
    AluOp op = static_cast<AluOp>(opcode.y);
    execute_alu_op(op, static_cast<uint8_t>(fetched_data));
}

void Cpu::handle_jp_and_interrupts() {
    switch (opcode.y) {
        case 0: // JP u16 (unconditional jump)
            regs.pc = fetched_data;
            break;
        case 6: // DI (0xF3) - Disable Interrupts
            ime = false;
            ime_delay = false;
            break;
        case 7: // EI (0xFB) - Enable Interrupts
            ime_delay = true; // Enable after next instruction
            break;
    }
}

void Cpu::handle_ret_conditional() {
    ConditionCode cc = static_cast<ConditionCode>(opcode.y);
    if (check_condition(cc)) 
    {
        regs.pc = stack_pop16();
        branch_taken = true;
    }
}

void Cpu::handle_pop_r16() {
    R16_Group3 reg = static_cast<R16_Group3>(opcode.y >> 1);
    uint16_t value = stack_pop16();
    set_r16_group3(reg, value);
}

void Cpu::handle_push_r16() {
    R16_Group3 reg = static_cast<R16_Group3>(opcode.y >> 1);
    uint16_t value = get_r16_group3(reg);
    stack_push16(value);
}

void Cpu::handle_call_conditional() {
    ConditionCode cc = static_cast<ConditionCode>(opcode.y);
    if (check_condition(cc)) {
        stack_push16(regs.pc);
        regs.pc = fetched_data;
        branch_taken = true;
    }
}

//  Instruction Group Handler x3 
void Cpu::execute_x3_instructions() {
    switch (opcode.z) {
        case 0: // RET conditional
            if (opcode.y < 4) 
                handle_ret_conditional();
            
            else if (opcode.y == 5)
            {
                // ADD SP, e8
                int8_t value = static_cast<int8_t>(fetched_data & 0xFF);
                uint16_t original = regs.sp;
                regs.sp += value;
                set_flag_c(((original & 0xFF) + (value & 0xFF)) > 0xFF); //We check carry on lower byte addition, not full 16 bit
                set_flag_h(((original & 0x0F) + (value & 0x0F)) > 0x0F);
                set_flag_n(false);
                set_flag_z(false);
            }
            else if (opcode.y == 6)
            {
                //LDH A, (u8)
                uint8_t addr = static_cast<uint8_t>(fetched_data);
                uint8_t value = bus->bus_read(0xFF00 + addr);
                write_r8(R8::A, value);
            }
            else if (opcode.y == 7)
            {
                //LD HL, SP+e8
                int8_t offset = static_cast<int8_t>(fetched_data & 0xFF);
                uint16_t original = regs.sp;
                set_r16_group1(R16_Group1::HL, original + offset);
                set_flag_z(false);
                set_flag_n(false);
                set_flag_h(((original & 0x0F) + (offset & 0x0F)) > 0x0F);
                set_flag_c(((original & 0xFF) + (offset & 0xFF)) > 0xFF);

            }
            else if (opcode.y == 4)
            {
                // LD (a8), A
                uint8_t addr = static_cast<uint8_t>(fetched_data);
                uint8_t value = read_r8(R8::A);
                bus->bus_write(0xFF00 + addr, value);
            }

            break;
            
        case 1: // POP r16 or RET/RETI/JP HL
            if (opcode.y & 1) {
                // RET, RETI, JP HL, LD SP,HL (odd y)
                switch (opcode.y >> 1) 
                {
                    case 0: // RET (0xC9)
                        regs.pc = stack_pop16();
                        break;
                    case 1: // RETI (0xD9)
                        regs.pc = stack_pop16();
                        ime = true;
                        break;
                    case 2: // JP HL (0xE9)
                        regs.pc = get_r16_group1(R16_Group1::HL);
                        break;
                    case 3: // LD SP, HL (0xF9)
                        regs.sp = get_r16_group1(R16_Group1::HL);
                        break;
                }
            } else {
                // POP r16 (even y)
                handle_pop_r16();
            }
            break;
            
        case 2: 
            if (opcode.y < 4) { // JP conditional
                ConditionCode cc = static_cast<ConditionCode>(opcode.y);
                if (check_condition(cc)) {
                    regs.pc = fetched_data;
                    branch_taken = true;
                }
            }
            else if (opcode.y == 6)
            {
                //LD A, (C)
                uint8_t addr = regs.c;
                uint8_t value = bus->bus_read(0xFF00 + addr);
                write_r8(R8::A, value);
            }
            else if (opcode.y == 7)
            {
                //LD A, (u16)
                uint8_t value = bus->bus_read(fetched_data);
                write_r8(R8::A, value);
                
            }
            else if (opcode.y == 5)
            {
                //LD (u16), A
                uint16_t addr = fetched_data;
                uint8_t value = read_r8(R8::A);
                bus->bus_write(addr, value);
            }
            else if (opcode.y == 4)
            {
                //LD (C), A
                uint8_t addr = regs.c;
                uint8_t value = read_r8(R8::A);
                bus->bus_write(0xFF00 + addr, value);
            }
            break;
            
        case 3: // JP and interrupt control
            handle_jp_and_interrupts();
            break;
            
        case 4: // CALL conditional
            if (opcode.y < 4) {
                handle_call_conditional();
            }
            break;
            
        case 5: // CALL
            if (opcode.y == 1) {
                stack_push16(regs.pc);
                regs.pc = fetched_data;
            }
                // Other odd y values: CB prefix or not used
             else {
                // PUSH r16 
                handle_push_r16();
            }
            break;
            
        case 6: // ALU A, u8
            handle_alu_imm8();
            break;
            
        case 7: // RST (restart)
            {
                uint8_t rst_addr = opcode.y * 8;
                stack_push16(regs.pc);
                regs.pc = rst_addr;
            }
            break;
    }
}
