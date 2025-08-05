#include "rom.h"

typedef struct components
{
    Rom rom; // Initialize Rom instance
} components_t;

class Emu
{
    public:
        static components_t cmp;
};