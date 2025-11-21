#include "cpu.h"
#include "../bus.h"

//  Specific Instruction Handlers for x0 group 
void Cpu::handle_ld_r16_imm16() {
    R16_Group1 reg = static_cast<R16_Group1>(opcode.y >> 1);
    set_r16_group1(reg, fetched_data);
}

void Cpu::handle_inc_r8() {
    R8 reg = static_cast<R8>(opcode.y);
    uint8_t original = read_r8(reg);
    uint8_t result = original + 1;
    write_r8(reg, result);
    set_flags_inc(result, original);
}

void Cpu::handle_dec_r8() {
    R8 reg = static_cast<R8>(opcode.y);
    uint8_t original = read_r8(reg);
    uint8_t result = original - 1;
    write_r8(reg, result);
    set_flags_dec(result, original);
}

void Cpu::handle_ld_r8_imm8() {
    R8 reg = static_cast<R8>(opcode.y);
    write_r8(reg, static_cast<uint8_t>(fetched_data));
}

void Cpu::handle_inc_r16() {
    R16_Group1 reg = static_cast<R16_Group1>(opcode.y >> 1);
    uint16_t value = get_r16_group1(reg);
    set_r16_group1(reg, value + 1);
}

void Cpu::handle_dec_r16() {
    R16_Group1 reg = static_cast<R16_Group1>(opcode.y >> 1);
    uint16_t value = get_r16_group1(reg);
    set_r16_group1(reg, value - 1);
}

void Cpu::handle_ld_indirect() {
    R16_Group2 reg = static_cast<R16_Group2>(opcode.y >> 1);
    uint16_t addr = get_r16_group2(reg);
    
    if (opcode.z == 2) {
        // LD (r16), A
        bus.bus_write(addr, regs.a);
    } else {
        // LD A, (r16)
        regs.a = bus.bus_read(addr);
    }
}

// ===== Instruction Group Handler x0 =====
void Cpu::execute_x0_instructions() {
    switch (opcode.z) {
        case 0: 
            if (opcode.y == 0) {
                // NOP
            }
            // Other y values: STOP, JR, etc. (to be implemented)
            break;
            
        case 1: // LD r16, u16 or ADD HL, r16
            if (opcode.y & 1) {
                // ADD HL, r16 (odd y values)
                R16_Group1 reg = static_cast<R16_Group1>(opcode.y >> 1);
                uint16_t hl = get_r16_group1(R16_Group1::HL);
                uint16_t value = get_r16_group1(reg);
                uint32_t result = hl + value; //Use 32 bit value here since we are dealing with 16 bit register
                
                set_flag_n(false);
                set_flag_h((hl & 0xFFF) + (value & 0xFFF) > 0xFFF);
                set_flag_c(result > 0xFFFF);
                set_r16_group1(R16_Group1::HL, result & 0xFFFF);
            } else {
                // LD r16, u16 (even y values)
                handle_ld_r16_imm16();
            }
            break;
            
        case 2: // LD (r16), A or LD A, (r16)
            handle_ld_indirect();
            break;
            
        case 3: // INC r16 or DEC r16
            if (opcode.y & 1) {
                handle_dec_r16();
            } else {
                handle_inc_r16();
            }
            break;
            
        case 4: // INC r8
            handle_inc_r8();
            break;
            
        case 5: // DEC r8
            handle_dec_r8();
            break;
            
        case 6: // LD r8, u8
            handle_ld_r8_imm8();
            break;
            
        case 7: // Accumulator/flag operations
            {
                AccFlagOp op = static_cast<AccFlagOp>(opcode.y);
                execute_acc_flag_op(op);
            }
            break;
    }
}
