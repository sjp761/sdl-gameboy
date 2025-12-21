#pragma once
#include <cstdint>
#include "memory_map.h"
#include <string>
class Timer; // Forward declaration
class Rom; // Forward declaration
class Ppu; // Forward declaration
class DMA;
class LCD;
class Bus
{
    private:
        Rom* rom;
        Timer* timer; // Timer reference
        Ppu* ppu; //  PPU reference
        DMA* dma; //  DMA reference
        LCD* lcd; //  LCD reference
        
    public:
        Bus();
        // Set component pointers
        void set_cmp(Rom* rom_ptr, Timer* timer_ptr, Ppu* ppu_ptr, DMA* dma_ptr, LCD* lcd_ptr) { rom = rom_ptr; timer = timer_ptr; ppu = ppu_ptr; dma = dma_ptr; lcd = lcd_ptr; }
        uint8_t bus_read(uint16_t address);
        void bus_write(uint16_t address, uint8_t data);
        void exram_write(uint16_t address, uint8_t value);
        void echoram_write(uint16_t address, uint8_t value);
        void wram_write(uint16_t address, uint8_t value);
        void vram_write(uint16_t address, uint8_t value);
        void io_write(uint16_t address, uint8_t value);
        void oam_write(uint16_t address, uint8_t value);
        uint8_t exram_read(uint16_t address);
        uint8_t echoram_read(uint16_t address);
        uint8_t wram_read(uint16_t address);
        uint8_t vram_read(uint16_t address);
        uint8_t io_read(uint16_t address);
        uint8_t oam_read(uint16_t address);
        uint8_t eram[MemoryMap::ERAM_SIZE] = {}; // 8KB External RAM (0xA000-0xBFFF)
        uint8_t wram[MemoryMap::WRAM_SIZE] = {}; // 8KB Work RAM (0xC000-0xDFFF)
        uint8_t io[MemoryMap::IO_SIZE] = {}; // I/O (0xFF00-0xFF7F)
        uint8_t high_ram[MemoryMap::HRAM_SIZE] = {}; // 127 bytes High RAM (0xFF80-0xFFFE)
        uint8_t ie_register = 0; // Interrupt Enable register (0xFFFF)
        uint8_t if_register = 0; // Interrupt Flag register (0xFF0F)
        std::string serial_buffer = "";
};