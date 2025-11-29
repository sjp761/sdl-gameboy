#include "bus.h"
#include "rom.h"
#include "timer.h"
#include <cstdint>
#include <cstdio>
#include <iostream>

Bus::Bus(Rom& rom_ref) : rom(rom_ref) {}

void Bus::attach_timer(Timer& timer_ref)
{
    timer = &timer_ref;
}

uint8_t Bus::bus_read(uint16_t address)
{
    if (MemoryMap::is_rom(address))
    {
        return rom.cart_read(address);
    }
    else if (MemoryMap::is_vram(address))
    {
        return vram_read(address);
    }
    else if (MemoryMap::is_eram(address))
    {
        return exram_read(address);
    }
    else if (MemoryMap::is_wram(address))
    {
        return wram_read(address);
    }
    else if (MemoryMap::is_echo(address))
    {
        return echoram_read(address);
    }
    else if (MemoryMap::is_oam_io(address))
    {
        return oam_io_read(address);
    }
    else if (MemoryMap::is_hram(address))
    {
        if (address == MemoryMap::IE_REGISTER) {
            return ie_register;
        }
        return high_ram[address - MemoryMap::HRAM_START];
    }
    return 0xFF; // Default for unmapped areas
}

void Bus::bus_write(uint16_t address, uint8_t data)
{
    if (MemoryMap::is_rom(address))
    {
        rom.cart_write(address, data);
    }
    else if (MemoryMap::is_vram(address))
    {
        vram_write(address, data);
    }
    else if (MemoryMap::is_eram(address))
    {
        exram_write(address, data);
    }
    else if (MemoryMap::is_wram(address))
    {
        wram_write(address, data);
    }
    else if (MemoryMap::is_echo(address))
    {
        echoram_write(address, data);
    }
    else if (MemoryMap::is_oam_io(address))
    {
        oam_io_write(address, data);
    }
    else if (MemoryMap::is_hram(address))
    {
        if (address == MemoryMap::IE_REGISTER) {
            ie_register = data;
        } else {
            high_ram[address - MemoryMap::HRAM_START] = data;
        }
    }
}

void Bus::vram_write(uint16_t address, uint8_t value)
{
    vram[address - MemoryMap::VRAM_START] = value;
}

void Bus::oam_io_write(uint16_t address, uint8_t value)
{
    // Route timer register writes to Timer component when attached
    if (timer && (address >= 0xFF04 && address <= 0xFF07)) {
        timer->write(address, value);
        return;
    }
    oam_io[address - MemoryMap::OAM_IO_START] = value;
}

uint8_t Bus::vram_read(uint16_t address)
{
    return vram[address - MemoryMap::VRAM_START];
}

uint8_t Bus::oam_io_read(uint16_t address)
{
    if (timer &&address >= 0xFF04 && address <= 0xFF07) 
        return timer->read(address);
    return oam_io[address - MemoryMap::OAM_IO_START];
}

void Bus::exram_write(uint16_t address, uint8_t value) //For External RAM
{
    eram[address - MemoryMap::ERAM_START] = value;
}

void Bus::echoram_write(uint16_t address, uint8_t value)
{
    wram_write((address - MemoryMap::ECHO_START) + MemoryMap::WRAM_START, value);
}

void Bus::wram_write(uint16_t address, uint8_t value)
{
    wram[address - MemoryMap::WRAM_START] = value;
}

uint8_t Bus::exram_read(uint16_t address)
{
    return eram[address - MemoryMap::ERAM_START];
}

uint8_t Bus::echoram_read(uint16_t address)
{
    return wram_read((address - MemoryMap::ECHO_START) + MemoryMap::WRAM_START);
}

uint8_t Bus::wram_read(uint16_t address)
{
    return wram[address - MemoryMap::WRAM_START];
}