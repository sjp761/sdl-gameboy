#include "timer.h"

Timer::Timer() : div(0xAC00), tima(0), tma(0), tac(0), bus(nullptr)
{
}

void Timer::falling_edge_check()
{

    // How I believe this is intended to work: the timer is effectively checking for an overflow from bit n to bit n+1
    // Ex: old_div = 0b1111 or 15, div = 0b10000 or 16: there is a carry from bit 3 to bit 4, signifying a virtual overflow (if the available bits would be [n:0]) meaning 16 t states have passed
    // Div register bug, whenever the div register is reset, it may cause a falling edge if the timer is enabled and the selected bit is high in the old div value
    uint16_t bit_mask = 0;
    switch (tac & 0b11) {
        case 0: bit_mask = 1 << 9; break;  // 4096 Hz
        case 1: bit_mask = 1 << 3; break;  // 262144 Hz
        case 2: bit_mask = 1 << 5; break;  // 65536 Hz
        case 3: bit_mask = 1 << 7; break;  // 16384 Hz
    }
    
    bool falling_edge = (old_div & bit_mask) && !(div & bit_mask);
    bool timer_enabled = (tac & 0b100);
    
    if (falling_edge && timer_enabled) {
        uint8_t prev = tima;
        tima++;
        if (prev == 0xFF) {
            tima = tma;
            if (bus) {
                bus->if_register |= static_cast<uint8_t>(Interrupts::InterruptMask::IT_Timer);
            }
        }
    }
}

uint8_t Timer::read(uint16_t address) const
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
            old_div = div;
            div = 0;
            falling_edge_check();
            break;
        case 0xFF05: // TIMA
            tima = value;
            break;
        case 0xFF06: // TMA
            tma = value;
            break;
        case 0xFF07: // TAC
            tac = value & 0b111; // Only lower 3 bits are used
            break;
        }
}

void Timer::tick()
{
    old_div = div;
    div++;
    falling_edge_check();
}
