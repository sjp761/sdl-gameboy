#include "timer.h"

Timer::Timer(Cpu& cpu_ref, Bus& bus_ref) : div(0), tima(0), tma(0), tac(0), cpu(cpu_ref), bus(bus_ref)
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
    uint16_t old_div = div;
    div++;
    
    // Check for falling edge on the selected bit
    int bit_position;
    switch (tac & 0x03)
    {
        case 0: //Hz here is how many times the timer increments per second
            bit_position = 9; // Bit 9 of DIV (1024 t-states, ~4096 Hz at 4.19 MHz clock)
            break;
        case 1: 
            bit_position = 3; // Bit 3 of DIV (16 t-states, ~262144 Hz at 4.19 MHz clock)
            break;
        case 2: 
            bit_position = 5; // Bit 5 of DIV (64 t-states, ~65536 Hz at 4.19 MHz clock)
            break;
        case 3: 
            bit_position = 7; // Bit 7 of DIV (256 t-states, ~16384 Hz at 4.19 MHz clock)
            break;
    }
    
    
    bool old_bit = (old_div >> bit_position) & 1;
    bool new_bit = (div >> bit_position) & 1;
    bool timer_enabled = tac & 0x04;
    
    // Increment TIMA on falling edge when timer is enabled
    if (timer_enabled && old_bit && !new_bit)
    {
        if (tima == 0xFF)
        {
            tima = 0; // Overflow to 0
            // TMA reload and interrupt request happen in the next cycle
            // For simplicity, doing it immediately here
            tima = tma;
            bus.if_register |= static_cast<uint8_t>(Interrupts::InterruptMask::IT_Timer);
        }
        else
        {
            tima++;
        }
    }
}
