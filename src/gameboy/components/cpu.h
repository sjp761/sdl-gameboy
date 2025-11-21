#pragma once
#include <cstdint>

struct Opcode
{
    uint8_t whole;
    uint8_t x;
    uint8_t y;
    uint8_t z;
    Opcode(uint8_t op) : whole(op), x((op >> 6) & 0x03), y((op >> 3) & 0x07), z(op & 0x07) {}
    Opcode() : whole(0), x(0), y(0), z(0) {}
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
        Opcode opcode;
        bool halted;
        bool stepping;
        bool ime; // Interrupt Master Enable flag
};

