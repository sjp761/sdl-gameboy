#include "rom.h"
#include <iostream>
#include <filesystem>
#include <fstream>
#include <cstring>
#include <iomanip>
#include "emu.h"

ROM::ROM(RomData& romData) // Because RomData contains unique_ptr, we need to use move semantics and and a reference (unique_ptr does not support copy)
{
    ctx = std::move(romData.ctx); // Move the cart_context from RomData to ROM, safe to use with unique_ptr
}

uint8_t ROM::cart_read(uint16_t addr)
{
    if (ctx.bootrom_enabled && addr < 0x0100) 
    {
        return ctx.bootrom_data[addr];
    }
    
    // If no ROM is loaded, return 0xFF (open bus behavior)
    if (!ctx.rom_data) {
        return 0xFF;
    }
    
    return ctx.rom_data[addr];
}

void ROM::cart_write(uint16_t addr, uint8_t value)
{
    // Default implementation does nothing, overridden by MBC classes
}

void ROM::disable_bootrom()
{
    ctx.bootrom_enabled = false;
    std::cout << "Bootrom disabled\n";
}
