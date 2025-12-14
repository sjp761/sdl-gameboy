#include "timer.h"

Timer::Timer() : div(0), tima(0), tma(0), tac(0), cpu(nullptr), bus(nullptr)
{
}

uint8_t Timer::read(uint16_t address)
{
    switch (address) {
        case 0xFF04: // DIV
            return static_cast<uint8_t>(div >> 8);
        case 0xFF05: // TIMA
            return tima;
        case 0xFF06: // TMA
            return tma;
        case 0xFF07: // TAC
            return tac;
        default:
            return 0xFF;
    }
}

void Timer::write(uint16_t address, uint8_t value)
{
    switch (address) {
        case 0xFF04: // DIV reset
            div = 0;
            break;
        case 0xFF05: // TIMA
            tima = value;
            break;
        case 0xFF06: // TMA
            tma = value;
            break;
        case 0xFF07: // TAC
            tac = value & 0b111; // only lower 3 bits used
            break;
        default:
            break;
    }
}

void Timer::tick()
{
    div++;
    
    if (!(tac & 0x04)) return; // Timer not enabled
    
    bool update_timer = false;
    switch (tac & 0x03)
    {
        case 0: // 1024 t-states
            update_timer = (div % 1024) == 0;
            break;
        case 1: // 16 t-states
            update_timer = (div % 16) == 0;
            break;
        case 2: // 64 t-states
            update_timer = (div % 64) == 0;
            break;
        case 3: // 256 t-states
            update_timer = (div % 256) == 0;
            break;
    }
    
    if (update_timer)
    {
        if (tima == 0xFF)
        {
            tima = 0;
            tima = tma;
            bus->if_register |= static_cast<uint8_t>(Interrupts::InterruptMask::IT_Timer);
        }
        else
        {
            tima++;
        }
    }
}
