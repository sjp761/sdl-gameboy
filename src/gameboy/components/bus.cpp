#include "bus.h"
#include "emu.h"
#include "rom.h"
#include <cstdint>

uint8_t Bus::bus_read(uint16_t address)
{
    if (address < 0x8000)
    {
        return Emu::cmp.rom.cart_read(address);
    }
    return 255;
}

void Bus::bus_write(uint16_t address, uint8_t data)
{
    if (address < 0x8000)
    {
        Emu::cmp.rom.cart_write(address, data);
    }
}