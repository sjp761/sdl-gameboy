#include "cpu.h"
#include "bus.h"

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
            break;
        case 7: // EI (0xFB) - Enable Interrupts
            ime = true;
            break;
    }
}

void Cpu::handle_ret_conditional() {
    ConditionCode cc = static_cast<ConditionCode>(opcode.y);
    if (check_condition(cc)) {
        uint8_t low = bus.bus_read(regs.sp++);
        uint8_t high = bus.bus_read(regs.sp++);
        regs.pc = (static_cast<uint16_t>(high) << 8) | low;
    }
}

void Cpu::handle_pop_r16() {
    R16_Group3 reg = static_cast<R16_Group3>(opcode.y >> 1);
    uint8_t low = bus.bus_read(regs.sp++);
    uint8_t high = bus.bus_read(regs.sp++);
    uint16_t value = (static_cast<uint16_t>(high) << 8) | low;
    set_r16_group3(reg, value);
}

void Cpu::handle_push_r16() {
    R16_Group3 reg = static_cast<R16_Group3>(opcode.y >> 1);
    uint16_t value = get_r16_group3(reg);
    bus.bus_write(--regs.sp, value >> 8);
    bus.bus_write(--regs.sp, value & 0xFF);
}

void Cpu::handle_call_conditional() {
    ConditionCode cc = static_cast<ConditionCode>(opcode.y);
    if (check_condition(cc)) {
        bus.bus_write(--regs.sp, regs.pc >> 8);
        bus.bus_write(--regs.sp, regs.pc & 0xFF);
        regs.pc = fetched_data;
    }
}

//  Instruction Group Handler x3 
void Cpu::execute_x3_instructions() {
    switch (opcode.z) {
        case 0: // RET conditional
            if (opcode.y < 4) {
                handle_ret_conditional();
            }
            // y >= 4: other operations (to be implemented)
            break;
            
        case 1: // POP r16 or RET/RETI/JP HL
            if (opcode.y & 1) {
                // RET, RETI, JP HL, LD SP,HL (odd y)
                switch (opcode.y) {
                    case 1: // RET (0xC9)
                        {
                            uint8_t low = bus.bus_read(regs.sp++);
                            uint8_t high = bus.bus_read(regs.sp++);
                            regs.pc = (static_cast<uint16_t>(high) << 8) | low;
                        }
                        break;
                    case 3: // (0xDB - not used)
                        break;
                    case 5: // (0xED - not used)
                        break;
                    case 7: // LD SP, HL (0xF9)
                        regs.sp = get_r16_group1(R16_Group1::HL);
                        break;
                }
            } else {
                // POP r16 (even y)
                handle_pop_r16();
            }
            break;
            
        case 2: // JP conditional
            if (opcode.y < 4) {
                ConditionCode cc = static_cast<ConditionCode>(opcode.y);
                if (check_condition(cc)) {
                    regs.pc = fetched_data;
                }
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
            
        case 5: // PUSH r16 or CALL
            if (opcode.y & 1) {
                // CALL (0xCD) for y=1
                if (opcode.y == 1) {
                    bus.bus_write(--regs.sp, regs.pc >> 8);
                    bus.bus_write(--regs.sp, regs.pc & 0xFF);
                    regs.pc = fetched_data;
                }
                // Other odd y values: CB prefix or not used
            } else {
                // PUSH r16 (even y)
                handle_push_r16();
            }
            break;
            
        case 6: // ALU A, u8
            handle_alu_imm8();
            break;
            
        case 7: // RST (restart)
            {
                uint8_t rst_addr = opcode.y * 8;
                bus.bus_write(--regs.sp, regs.pc >> 8);
                bus.bus_write(--regs.sp, regs.pc & 0xFF);
                regs.pc = rst_addr;
            }
            break;
    }
}
