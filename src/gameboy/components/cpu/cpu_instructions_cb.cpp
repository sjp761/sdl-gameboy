#include "cpu.h"
#include "bus.h"

//  Instruction Group Handler CB 
void Cpu::execute_cb_instructions() {
    uint8_t cb_opcode = static_cast<uint8_t>(fetched_data);
    Opcode cb_op(cb_opcode);
    R8 reg = static_cast<R8>(cb_op.z);
    
    switch (cb_op.x) {
        case 0: // Shift/rotate operations
            {
                ShiftRotateOp op = static_cast<ShiftRotateOp>(cb_op.y);
                execute_shift_rotate_op(op, reg);
            }
            break;
            
        case 1: // BIT b, r8
            {
                uint8_t bit = cb_op.y;
                uint8_t value = read_r8(reg);
                set_flag_z(!(value & (1 << bit)));
                set_flag_n(false);
                set_flag_h(true);
                // Carry flag unchanged
            }
            break;
            
        case 2: // RES b, r8
            {
                uint8_t bit = cb_op.y;
                uint8_t value = read_r8(reg);
                value &= ~(1 << bit);
                write_r8(reg, value);
            }
            break;
            
        case 3: // SET b, r8
            {
                uint8_t bit = cb_op.y;
                uint8_t value = read_r8(reg);
                value |= (1 << bit);
                write_r8(reg, value);
            }
            break;
    }
}
