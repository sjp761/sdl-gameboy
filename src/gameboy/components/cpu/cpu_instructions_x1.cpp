#include "cpu.h"
#include "bus.h"

//  Specific Instruction Handler for x1 group 
void Cpu::handle_ld_r8_r8() {
    R8 dst = static_cast<R8>(opcode.y);
    R8 src = static_cast<R8>(opcode.z);
    
    if (dst == R8::HL_IND && src == R8::HL_IND) {
        // HALT instruction (0x76)
        halted = true;
    } else {
        write_r8(dst, read_r8(src));
    }
}

//  Instruction Group Handler x1 
void Cpu::execute_x1_instructions() {
    // LD r8, r8 (or HALT if y=6 and z=6)
    handle_ld_r8_r8();
}
