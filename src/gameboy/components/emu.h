#include "rom.h"
#include "cpu.h"

typedef struct components
{
    Rom rom; // Initialize Rom instance
    Cpu cpu;
} components_t;


struct emu_context 
{
    bool paused;
    bool running;
    uint64_t ticks;
};
    
class Emu
{
    public:
        static components_t cmp;
        static emu_context ctx;
};