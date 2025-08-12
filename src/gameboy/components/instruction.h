#pragma once
#include <cstdint>
#include <string>
#include <unordered_map>
struct Opcode;

// Addressing modes for Game Boy instructions.
// Each mode describes how operands are accessed or interpreted.
// Examples are given for each mode using typical Game Boy assembly syntax.
enum oprnd_desc {
    AM_IMP,        // Implied (no operand)
    AM_R,          // Register
    AM_R_R,        // Register, Register
    AM_R_IMM8,     // Register, 8-bit immediate
    AM_R_IMM16,    // Register, 16-bit immediate
    AM_R_MEM,      // Register, Memory
    AM_MEM_R,      // Memory, Register
    AM_MEM_IMM8,   // Memory, 8-bit immediate
    AM_MEM,        // Memory (no register)
    AM_R_ADDR,     // Register, Address (16-bit)
    AM_ADDR_R      // Address (16-bit), Register
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

// Forward declaration for Opcode if not already defined
class Opcode;

struct Instruction 
{
    oprnd_desc oprnd;
    in_type type;
    reg_type reg1;// Registers will not always be used
    reg_type reg2; 
    uint16_t data;
    int hl_increment = 0; // -1 for decrement, 1 for increment, 0 for nothing
    bool is_high_ram = false; // True if the instruction accesses high RAM (0xFF80-0xFFFF)
    Instruction() = default;
    Instruction(Opcode opcode);
    // std::string inst_name(in_type t);
};
