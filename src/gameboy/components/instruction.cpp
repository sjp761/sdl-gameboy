#include <unordered_map>
#include "instruction.h"

std::unordered_map<uint8_t, Instruction> InstructionMap {
    { 0x00, { IN_NOP, AM_IMP, RT_NONE, RT_NONE, CT_NONE, 0 } },
    { 0x01, { IN_LD, AM_R_N16, RT_BC, RT_NONE, CT_NONE, 0 } },
    { 0x02, { IN_LD, AM_MR_R, RT_A, RT_BC, CT_NONE, 0 } },
};

Instruction *Instruction::instruction_by_opcode(uint8_t opcode)
{
    auto it = InstructionMap.find(opcode);
    if (it != InstructionMap.end()) {
        return &it->second;
    }
    return nullptr; // or handle as you wish
}