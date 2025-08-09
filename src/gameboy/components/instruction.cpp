#include <unordered_map>
#include "instruction.h"
#include "cpu.h"

Instruction::Instruction(Opcode opcode)
{
    if (opcode.whole == 0x00) 
    {
        type = IN_NOP;
        return;
    }
    // Handle 0x00xx opcodes
    else if (opcode.x() == 0x00 && opcode.z() == 0x00 && opcode.y() == 1)
    {  // LD A16,SP
        type = IN_LD;
        reg1 = RT_SP;
        oprnd = AM_A16_R;
        return;
    }
    else
    {
        type = IN_NONE;
        return;
    }

}
