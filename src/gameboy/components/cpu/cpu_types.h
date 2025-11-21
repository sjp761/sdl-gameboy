#pragma once
#include <cstdint>

// Register enums for encoding
enum class R8 : uint8_t {
    B = 0, C = 1, D = 2, E = 3,
    H = 4, L = 5, HL_IND = 6, A = 7
};

enum class R16_Group1 : uint8_t {
    BC = 0, DE = 1, HL = 2, SP = 3
};

enum class R16_Group2 : uint8_t {
    BC = 0, DE = 1, HL_INC = 2, HL_DEC = 3
};

enum class R16_Group3 : uint8_t {
    BC = 0, DE = 1, HL = 2, AF = 3
};

enum class ConditionCode : uint8_t {
    NZ = 0, Z = 1, NC = 2, C = 3
};

enum class AluOp : uint8_t {
    ADD = 0, ADC = 1, SUB = 2, SBC = 3,
    AND = 4, XOR = 5, OR = 6, CP = 7
};

enum class AccFlagOp : uint8_t {
    RLCA = 0, RRCA = 1, RLA = 2, RRA = 3,
    DAA = 4, CPL = 5, SCF = 6, CCF = 7
};

enum class ShiftRotateOp : uint8_t {
    RLC = 0, RRC = 1, RL = 2, RR = 3,
    SLA = 4, SRA = 5, SWAP = 6, SRL = 7
};
