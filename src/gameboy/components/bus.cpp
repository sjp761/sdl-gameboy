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
    else if (address >= 0xA000 && address <= 0xBFFF)
    {
        return exram_read(address);
    }
    else if (address >= 0xC000 && address <= 0xDFFF)
    {
        return wram_read(address);
    }
    else if (address >= 0xE000 && address <= 0xFDFF)
    {
        return echoram_read(address);
    }
    return 255; // Default for unmapped areas
}

void Bus::bus_write(uint16_t address, uint8_t data)
{
    if (address < 0x8000)
    {
        Emu::cmp.rom.cart_write(address, data);
    }
    else if (address >= 0xA000 && address <= 0xBFFF)
    {
        exram_write(address, data);
    }
    else if (address >= 0xC000 && address <= 0xDFFF)
    {
        wram_write(address, data);
    }
    else if (address >= 0xE000 && address <= 0xFDFF)
    {
        echoram_write(address, data);
    }
}

void Bus::exram_write(uint16_t address, uint8_t value) //For External RAM
{
    eram[address & 0x1FFF] = value;
}

void Bus::echoram_write(uint16_t address, uint8_t value)
{
    wram_write((address - 0xE000) + 0xC000, value);
}

void Bus::wram_write(uint16_t address, uint8_t value)
{
    wram[(address - 0xC000) & 0x1FFF] = value;
}

uint8_t Bus::exram_read(uint16_t address)
{
    return eram[address & 0x1FFF];
}

uint8_t Bus::echoram_read(uint16_t address)
{
    return wram_read((address - 0xE000) + 0xC000);
}

uint8_t Bus::wram_read(uint16_t address)
{
    return wram[(address - 0xC000) & 0x1FFF];
}
