#include "cpu.h"
#include "bus.h"

// ===== ALU Operation Helper =====
void Cpu::execute_alu_op(AluOp op, uint8_t operand) {
    uint16_t result; //Using uint_16 here makes it much easier to detect a carry and half carry, ALU only deals with 8 bit registers so this works here
    uint8_t carry_in; //For carry, check if result is > 0xFF (More than 8 bytes) for overflow
    // For half carry. check for overflow from 4 bytes
    switch (op) {
        case AluOp::ADD:
            result = regs.a + operand;
            set_flag_z((result & 0xFF) == 0);
            set_flag_n(false);
            set_flag_h(((regs.a & 0x0F) + (operand & 0x0F)) > 0x0F);
            set_flag_c(result > 0xFF);
            regs.a = result & 0xFF;
            break;
            
        case AluOp::ADC:
            carry_in = get_flag_c() ? 1 : 0;
            result = regs.a + operand + carry_in;
            set_flag_z((result & 0xFF) == 0);
            set_flag_n(false);
            set_flag_h(((regs.a & 0x0F) + (operand & 0x0F) + carry_in) > 0x0F);
            set_flag_c(result > 0xFF);
            regs.a = result & 0xFF;
            break;
            
        case AluOp::SUB:
            result = regs.a - operand;
            set_flag_z((result & 0xFF) == 0);
            set_flag_n(true);
            set_flag_h((regs.a & 0x0F) < (operand & 0x0F));
            set_flag_c(regs.a < operand);
            regs.a = result & 0xFF;
            break;
            
        case AluOp::SBC:
            carry_in = get_flag_c() ? 1 : 0;
            result = regs.a - operand - carry_in;
            set_flag_z((result & 0xFF) == 0);
            set_flag_n(true);
            set_flag_h((int)(regs.a & 0x0F) - (int)(operand & 0x0F) - carry_in < 0);
            set_flag_c((int)regs.a - (int)operand - carry_in < 0);
            regs.a = result & 0xFF;
            break;
            
        case AluOp::AND:
            regs.a &= operand;
            set_flag_z(regs.a == 0);
            set_flag_n(false);
            set_flag_h(true);
            set_flag_c(false);
            break;
            
        case AluOp::XOR:
            regs.a ^= operand;
            set_flag_z(regs.a == 0);
            set_flag_n(false);
            set_flag_h(false);
            set_flag_c(false);
            break;
            
        case AluOp::OR:
            regs.a |= operand;
            set_flag_z(regs.a == 0);
            set_flag_n(false);
            set_flag_h(false);
            set_flag_c(false);
            break;
            
        case AluOp::CP:
            result = regs.a - operand;
            set_flag_z((result & 0xFF) == 0);
            set_flag_n(true);
            set_flag_h((regs.a & 0x0F) < (operand & 0x0F));
            set_flag_c(regs.a < operand);
            // Note: A is not modified
            break;
    }
}

// ===== Accumulator/Flag Operation Helper =====
void Cpu::execute_acc_flag_op(AccFlagOp op) {
    uint8_t carry;
    
    switch (op) {
        case AccFlagOp::RLCA: // Rotate left
            carry = (regs.a & 0x80) >> 7;
            regs.a = (regs.a << 1) | carry;
            set_flag_z(false);
            set_flag_n(false);
            set_flag_h(false);
            set_flag_c(carry != 0);
            break;
            
        case AccFlagOp::RRCA: // Rotate right
            carry = regs.a & 0x01;
            regs.a = (regs.a >> 1) | (carry << 7);
            set_flag_z(false);
            set_flag_n(false);
            set_flag_h(false);
            set_flag_c(carry != 0);
            break;
            
        case AccFlagOp::RLA: // Rotate left through carry
            carry = (regs.a & 0x80) >> 7;
            regs.a = (regs.a << 1) | (get_flag_c() ? 1 : 0);
            set_flag_z(false);
            set_flag_n(false);
            set_flag_h(false);
            set_flag_c(carry != 0);
            break;
            
        case AccFlagOp::RRA: // Rotate right through carry
            carry = regs.a & 0x01;
            regs.a = (regs.a >> 1) | (get_flag_c() ? 0x80 : 0);
            set_flag_z(false);
            set_flag_n(false);
            set_flag_h(false);
            set_flag_c(carry != 0);
            break;
            
        case AccFlagOp::DAA: // Decimal adjust accumulator
            // TODO: Implement DAA logic
            break;
            
        case AccFlagOp::CPL: // Complement A
            regs.a = ~regs.a;
            set_flag_n(true);
            set_flag_h(true);
            break;
            
        case AccFlagOp::SCF: // Set carry flag
            set_flag_n(false);
            set_flag_h(false);
            set_flag_c(true);
            break;
            
        case AccFlagOp::CCF: // Complement carry flag
            set_flag_n(false);
            set_flag_h(false);
            set_flag_c(!get_flag_c());
            break;
    }
}

// ===== Shift/Rotate Operation Helper (CB prefix) =====
void Cpu::execute_shift_rotate_op(ShiftRotateOp op, R8 reg) {
    uint8_t value = read_r8(reg);
    uint8_t carry;
    
    switch (op) {
        case ShiftRotateOp::RLC: // Rotate left
            carry = (value & 0x80) >> 7;
            value = (value << 1) | carry;
            set_flag_z(value == 0);
            set_flag_n(false);
            set_flag_h(false);
            set_flag_c(carry != 0);
            break;
            
        case ShiftRotateOp::RRC: // Rotate right
            carry = value & 0x01;
            value = (value >> 1) | (carry << 7);
            set_flag_z(value == 0);
            set_flag_n(false);
            set_flag_h(false);
            set_flag_c(carry != 0);
            break;
            
        case ShiftRotateOp::RL: // Rotate left through carry
            carry = (value & 0x80) >> 7;
            value = (value << 1) | (get_flag_c() ? 1 : 0);
            set_flag_z(value == 0);
            set_flag_n(false);
            set_flag_h(false);
            set_flag_c(carry != 0);
            break;
            
        case ShiftRotateOp::RR: // Rotate right through carry
            carry = value & 0x01;
            value = (value >> 1) | (get_flag_c() ? 0x80 : 0);
            set_flag_z(value == 0);
            set_flag_n(false);
            set_flag_h(false);
            set_flag_c(carry != 0);
            break;
            
        case ShiftRotateOp::SLA: // Shift left arithmetic
            carry = (value & 0x80) >> 7;
            value <<= 1;
            set_flag_z(value == 0);
            set_flag_n(false);
            set_flag_h(false);
            set_flag_c(carry != 0);
            break;
            
        case ShiftRotateOp::SRA: // Shift right arithmetic (preserve sign)
            carry = value & 0x01;
            value = (value >> 1) | (value & 0x80);
            set_flag_z(value == 0);
            set_flag_n(false);
            set_flag_h(false);
            set_flag_c(carry != 0);
            break;
            
        case ShiftRotateOp::SWAP: // Swap nibbles
            value = ((value & 0x0F) << 4) | ((value & 0xF0) >> 4);
            set_flag_z(value == 0);
            set_flag_n(false);
            set_flag_h(false);
            set_flag_c(false);
            break;
            
        case ShiftRotateOp::SRL: // Shift right logical
            carry = value & 0x01;
            value >>= 1;
            set_flag_z(value == 0);
            set_flag_n(false);
            set_flag_h(false);
            set_flag_c(carry != 0);
            break;
    }
    
    write_r8(reg, value);
}
