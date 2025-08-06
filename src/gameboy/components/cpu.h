#include <cstdint>
#include "instruction.h"

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
        cpu_registers regs;
        uint16_t fetch_data;
        uint16_t mem_dest;
        Instruction *current_instruction;
        uint8_t opcode;
        bool halted;
        bool stepping;
};