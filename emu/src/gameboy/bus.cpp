#include "bus.h"
#include "rom.h"
#include "timer.h"
#include "ppu.h"
#include "dma.h"
#include <cstdint>
#include <cstdio>
#include <iostream>
#include "lcd.h"

Bus::Bus() : rom(nullptr), timer(nullptr), ppu(nullptr), ie_register(0), if_register(0), test_mode(false)
{
    init_memory_table();
}

Bus::Bus(bool test_mode_enable) : rom(nullptr), timer(nullptr), ppu(nullptr), ie_register(0), if_register(0), test_mode(test_mode_enable)
{
    // Blank
}

void Bus::init_memory_table()
{
    memory_regions = {{
        {MemoryMap::ROM_BANK_00_START, MemoryMap::ROM_BANK_NN_END, &Bus::rom_read, &Bus::rom_write},
        {MemoryMap::VRAM_START, MemoryMap::VRAM_END, &Bus::vram_read, &Bus::vram_write},
        {MemoryMap::ERAM_START, MemoryMap::ERAM_END, &Bus::exram_read, &Bus::exram_write},
        {MemoryMap::WRAM_START, MemoryMap::WRAM_END, &Bus::wram_read, &Bus::wram_write},
        {MemoryMap::ECHO_START, MemoryMap::ECHO_END, &Bus::echoram_read, &Bus::echoram_write},
        {MemoryMap::OAM_START, MemoryMap::OAM_END, &Bus::oam_read, &Bus::oam_write},
        {MemoryMap::LCD_START, MemoryMap::LCD_END, &Bus::lcd_read, &Bus::lcd_write},
        {MemoryMap::IO_START, MemoryMap::IO_END, &Bus::io_read, &Bus::io_write},
        {MemoryMap::HRAM_START, MemoryMap::IE_REGISTER, &Bus::hram_read, &Bus::hram_write}
    }};
}


uint8_t Bus::bus_read(uint16_t address)
{
    #ifdef OPCODE_TEST
        return opcode_test_mem[address];
    #endif

    for (const auto& region : memory_regions) {
        if (address >= region.start && address <= region.end) {
            return (this->*region.read_fn)(address);
        }
    }
    return 0xFF; // Default for unmapped areas
}

void Bus::bus_write(uint16_t address, uint8_t data)
{
    #ifdef OPCODE_TEST
        opcode_test_mem[address] = data;
        return;
    #endif
    
    for (const auto& region : memory_regions) {
        if (address >= region.start && address <= region.end) {
            (this->*region.write_fn)(address, data);
            return;
        }
    }
}

void Bus::vram_write(uint16_t address, uint8_t value)
{
    if (ppu)
    {
        ppu->vram_write(address, value);
    }
}

void Bus::io_write(uint16_t address, uint8_t value)
{
    // Bootrom disable register (0xFF50) - write to ROM component
    if (address == 0xFF50 && rom) {
        std::cout << "Bus::io_write: write to 0xFF50, value=" << std::hex << (int)value << std::dec << std::endl;
        rom->disable_bootrom();
        // Don't store in io array, this is write-only
        return;
    }
    
    // Route timer register writes to Timer component when attached
    if (timer && (address >= 0xFF04 && address <= 0xFF07)) {
        timer->write(address, value);
        return;
    }
    // Interrupt Flag (IF) register at 0xFF0F is special: store in if_register
    if (address == MemoryMap::IF_REGISTER) {
        if_register = value;
        return;
    }

    if (address == 0xFF46 && dma) {
        // Start DMA transfer
        dma->start(value);
        return;
    }
    
    // Route audio register writes
    if ((address >= MemoryMap::AUDIO_START && address <= MemoryMap::AUDIO_END) ||
        (address >= MemoryMap::WAVE_RAM_START && address <= MemoryMap::WAVE_RAM_END)) {
        audio_write(address, value);
        return;
    }
    
    io[address - MemoryMap::IO_START] = value;
}

void Bus::oam_write(uint16_t address, uint8_t value)
{
    if (ppu)
    {
        ppu->oam_write(address, value);
    }
}

uint8_t Bus::vram_read(uint16_t address)
{
    if (ppu)
    {
        return ppu->vram_read(address);
    }
    return 0xFF;
}

uint8_t Bus::io_read(uint16_t address)
{
    if (timer && address >= 0xFF04 && address <= 0xFF07) 
        return timer->read(address);
    // Interrupt Flag (IF) register at 0xFF0F: read from if_register
    if (address == MemoryMap::IF_REGISTER) {
        return if_register;
    }
    
    // Route audio register reads
    if ((address >= MemoryMap::AUDIO_START && address <= MemoryMap::AUDIO_END) ||
        (address >= MemoryMap::WAVE_RAM_START && address <= MemoryMap::WAVE_RAM_END)) {
        return audio_read(address);
    }
    
    return io[address - MemoryMap::IO_START];
}

uint8_t Bus::oam_read(uint16_t address)
{
    if (ppu)
    {
        return ppu->oam_read(address);
    }
    return 0xFF;
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

uint8_t Bus::rom_read(uint16_t address)
{
    return rom->cart_read(address);
}

void Bus::rom_write(uint16_t address, uint8_t value)
{
    rom->cart_write(address, value);
}

uint8_t Bus::hram_read(uint16_t address)
{
    if (address == MemoryMap::IE_REGISTER) {
        return ie_register;
    }
    return high_ram[address - MemoryMap::HRAM_START];
}

void Bus::hram_write(uint16_t address, uint8_t value)
{
    if (address == MemoryMap::IE_REGISTER) {
        ie_register = value;
    } else {
        high_ram[address - MemoryMap::HRAM_START] = value;
    }
}

uint8_t Bus::lcd_read(uint16_t address)
{
    return lcd->lcd_read(address);
}

void Bus::lcd_write(uint16_t address, uint8_t value)
{
    lcd->lcd_write(address, value);
}

uint8_t Bus::audio_read(uint16_t address)
{
    // Wave Pattern RAM (0xFF30-0xFF3F)
    if (address >= MemoryMap::WAVE_RAM_START && address <= MemoryMap::WAVE_RAM_END) {
        return wave_ram[address - MemoryMap::WAVE_RAM_START];
    }
    
    // Audio registers (0xFF10-0xFF26)
    if (address >= MemoryMap::AUDIO_START && address <= MemoryMap::AUDIO_END) {
        return audio_regs[address - MemoryMap::AUDIO_START];
    }
    
    return 0xFF;
}

void Bus::audio_write(uint16_t address, uint8_t value)
{
    // Wave Pattern RAM (0xFF30-0xFF3F)
    if (address >= MemoryMap::WAVE_RAM_START && address <= MemoryMap::WAVE_RAM_END) {
        wave_ram[address - MemoryMap::WAVE_RAM_START] = value;
        return;
    }
    
    // Audio registers (0xFF10-0xFF26)
    if (address >= MemoryMap::AUDIO_START && address <= MemoryMap::AUDIO_END) {
        audio_regs[address - MemoryMap::AUDIO_START] = value;
        return;
    }
}