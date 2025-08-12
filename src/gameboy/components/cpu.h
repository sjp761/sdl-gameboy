#pragma once
#include <cstdint>
#include "instruction.h" // Ensure this header defines the full Instruction type
#include <json/json.h>

struct Opcode
{
    uint8_t whole;
    uint8_t x() const { return (whole >> 6) & 0x03; }
    uint8_t y() const { return (whole >> 3) & 0x07; }
    uint8_t z() const { return whole & 0x07; }
    Opcode(uint8_t op) : whole(op) {}
    Opcode() : whole(0) {}
};

struct cpu_registers
{
    uint8_t a;
    uint8_t f;
    uint8_t b;
    uint8_t c;
    uint8_t d;
    uint8_t e;
    uint8_t h;
    uint8_t l;
    uint16_t pc;
    uint16_t sp;
};

class Cpu
{
    public:
        void cpu_init();
        bool cpu_step();
        void fetch_data();
        void emu_cycles();
        void fetch_instruction();
        void execute_instruction();
        cpu_registers regs;
        uint16_t fetched_data;
        uint16_t mem_dest;
        Instruction instruction;
        Opcode opcode;
        bool halted;
        bool stepping;

#ifdef OPCODETEST
        void set_opcode_test_data(Json::Value& root, int index);
        bool check_opcode_data(Json::Value &root, int index);
#endif
};

