#pragma once
#include <cstdint>
#include <string>

// Addressing modes for Game Boy instructions.
// Each mode describes how operands are accessed or interpreted.
// Examples are given for each mode using typical Game Boy assembly syntax.
enum oprnd_desc
{
    AM_IMP,      // Implied: No operand. Example: NOP
    AM_R_N16,    // Register, 16-bit immediate. Example: LD BC, 0x1234
    AM_R_R,      // Register, Register. Example: LD A, B
    AM_MR_R,     // Memory at register, Register. Example: LD (HL), A
    AM_R,        // Single register operand. Example: INC B
    AM_R_N8,     // Register, 8-bit immediate. Example: LD B, 0x12
    AM_R_MR,     // Register, Memory at register. Example: LD A, (HL)
    AM_R_HLI,    // Register, (HL+) (post-increment HL). Example: LD A, (HL+)
    AM_R_HLD,    // Register, (HL-) (post-decrement HL). Example: LD A, (HL-)
    AM_HLI_R,    // (HL+), Register. Example: LD (HL+), A
    AM_HLD_R,    // (HL-), Register. Example: LD (HL-), A
    AM_R_A8,     // Register, 8-bit address (high RAM). Example: LD A, (0xFF00 + n)
    AM_A8_R,     // 8-bit address (high RAM), Register. Example: LD (0xFF00 + n), A
    AM_HL_SPR,   // HL, SP + signed immediate. Example: LD HL, SP+e
    AM_N16,      // 16-bit immediate. Example: JP 0x1234
    AM_N8,       // 8-bit immediate. Example: ADD A, 0x12
    AM_N16_R,    // 16-bit immediate, Register. Example: LD (0x1234), SP
    AM_MR_N8,    // Memory at register, 8-bit immediate. Example: LD (HL), 0x12
    AM_MR,       // Memory at register. Example: INC (HL)
    AM_A16_R,    // 16-bit address, Register. Example: LD (0x1234), A
    AM_R_A16     // Register, 16-bit address. Example: LD A, (0x1234)
};

enum reg_type {
    RT_NONE,
    RT_A,
    RT_F,
    RT_B,
    RT_C,
    RT_D,
    RT_E,
    RT_H,
    RT_L,
    RT_AF, // 2 byte registers
    RT_BC,
    RT_DE,
    RT_HL,
    RT_SP,
    RT_PC
};

enum in_type {
    IN_NONE,
    IN_NOP,
    IN_LD,
    IN_INC,
    IN_DEC,
    IN_RLCA,
    IN_ADD,
    IN_RRCA,
    IN_STOP,
    IN_RLA,
    IN_JR,
    IN_RRA,
    IN_DAA,
    IN_CPL,
    IN_SCF,
    IN_CCF,
    IN_HALT,
    IN_ADC,
    IN_SUB,
    IN_SBC,
    IN_AND,
    IN_XOR,
    IN_OR,
    IN_CP,
    IN_POP,
    IN_JP,
    IN_PUSH,
    IN_RET,
    IN_CB,
    IN_CALL,
    IN_RETI,
    IN_LDH,
    IN_JPHL,
    IN_DI,
    IN_EI,
    IN_RST,
    IN_ERR,
    //CB instructions...
    IN_RLC, 
    IN_RRC,
    IN_RL, 
    IN_RR,
    IN_SLA, 
    IN_SRA,
    IN_SWAP, 
    IN_SRL,
    IN_BIT, 
    IN_RES, 
    IN_SET
};

enum cond_type {
    CT_NONE, CT_NZ, CT_Z, CT_NC, CT_C
};

class Instruction {
public:
    in_type type;
    oprnd_desc desc;
    reg_type reg_1;
    reg_type reg_2; // Not all registers will be used always
    cond_type cond;
    uint8_t param;
    Instruction *instruction_by_opcode(uint8_t opcode);
    std::string inst_name(in_type t);


    Instruction()
        : type(IN_NONE), desc(AM_IMP), reg_1(RT_NONE), reg_2(RT_NONE), cond(CT_NONE), param(0) {}

    Instruction(in_type t, oprnd_desc m, reg_type r1, reg_type r2, cond_type c, uint8_t p)
        : type(t), desc(m), reg_1(r1), reg_2(r2), cond(c), param(p) {}
    

};