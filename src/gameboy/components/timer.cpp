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
    uint16_t old_div = div++;
    bool update_timer = false;
    switch (tac & 0x03)
    {
        case 0: // 4096 Hz or 256 M-cycles
            update_timer = (old_div & 0xFF) == 0xFF;
            break;
        case 1: // 262144 Hz or 4 M-cycles
            update_timer = (old_div & 0x03) == 0x03;
            break;
        case 2: // 65536 Hz or 16 M-cycles  
            update_timer = (old_div & 0x0F) == 0x0F;
            break;
        case 3: // 16384 Hz or 64 M-cycles
            update_timer = (old_div & 0x3F) == 0x3F;
            break;
    }
    if (update_timer && (tac & 0b100))
    {
        tima++;
        if (tima == 0xFF)
        {
            tima = tma; //Set to module if overflows
            bus.if_register |= static_cast<uint8_t>(Interrupts::InterruptMask::IT_Timer); //Request timer interrupt
        }
    }
}
