#include "timer.h"

Timer::Timer() : div(0xAC00), tima(0), tma(0), tac(0), bus(nullptr)
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
            old_div = div;
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
    uint16_t old_div = div++;

    bool timer_update = false; // How I believe this is intended to work: the timer is effectively checking for an overflow from bit n to bit n+1
    switch (tac & 0b11) { // Ex: old_div = 0b1111 or 15, div = 0b10000 or 16: there is a carry from bit 3 to bit 4, signifying a virtual overflow (if the available bits would be [n:0]) meaning 16 t states have passed
        case 0: // 4096 Hz 
            // Falling edge of bit 9 (from 1->0)
            timer_update = (old_div & (1 << 9)) && !(div & (1 << 9)); // There is a quirk where tima increments when div is 0 and old div has the specific bit enabled so check this way
            break;
        case 1: // 262144 Hz
            // Falling edge of bit 3
            timer_update = (old_div & (1 << 3)) && !(div & (1 << 3));
            break;
        case 2: // 65536 Hz
            // Falling edge of bit 5
            timer_update = (old_div & (1 << 5)) && !(div & (1 << 5));
            break;
        case 3: // 16384 Hz
            // Falling edge of bit 7
            timer_update = (old_div & (1 << 7)) && !(div & (1 << 7));
            break;
    }

    if ((tac & 0b100) && timer_update) 
    {
        uint8_t prev = tima;
        tima++;
        // TIMA overflow occurs when incrementing 0xFF -> 0x00
        if (prev == 0xFF) 
        {
            tima = tma;
            // Request Timer Interrupt
            if (bus) {
                bus->if_register |= static_cast<uint8_t>(Interrupts::InterruptMask::IT_Timer);
            }
        }
    }
}
