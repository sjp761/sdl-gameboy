#include "cpu.h"
#include "bus.h"

//  Specific Instruction Handler for x2 group 
void Cpu::handle_alu_r8() {
    R8 src = static_cast<R8>(opcode.z);
    AluOp op = static_cast<AluOp>(opcode.y);
    uint8_t operand = read_r8(src);
    execute_alu_op(op, operand);
}

//  Instruction Group Handler x2 
void Cpu::execute_x2_instructions() {
    // ALU A, r8
    handle_alu_r8();
}
