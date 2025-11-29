#pragma once
#include <cstdint>
#include "memory_map.h"
#include <string>
class Timer; // Forward declaration
class Rom; // Forward declaration

class Bus
{
    private:
        Rom& rom;
        Timer* timer = nullptr; // Optional timer reference
        
    public:
        Bus(Rom& rom_ref);
        void attach_timer(Timer& timer_ref);
        uint8_t bus_read(uint16_t address);
        void bus_write(uint16_t address, uint8_t data);
        void exram_write(uint16_t address, uint8_t value);
        void echoram_write(uint16_t address, uint8_t value);
        void wram_write(uint16_t address, uint8_t value);
        void vram_write(uint16_t address, uint8_t value);
        void oam_io_write(uint16_t address, uint8_t value);
        uint8_t exram_read(uint16_t address);
        uint8_t echoram_read(uint16_t address);
        uint8_t wram_read(uint16_t address);
        uint8_t vram_read(uint16_t address);
        uint8_t oam_io_read(uint16_t address);
        uint8_t eram[MemoryMap::ERAM_SIZE] = {}; // 8KB External RAM (0xA000-0xBFFF)
        uint8_t wram[MemoryMap::WRAM_SIZE] = {}; // 8KB Work RAM (0xC000-0xDFFF)
        uint8_t vram[MemoryMap::VRAM_SIZE] = {}; // 8KB Video RAM (0x8000-0x9FFF)
        uint8_t oam_io[MemoryMap::OAM_IO_SIZE] = {}; // OAM + I/O (0xFE00-0xFF7F)
        uint8_t high_ram[MemoryMap::HRAM_SIZE] = {}; // 127 bytes High RAM (0xFF80-0xFFFE)
        uint8_t ie_register = 0; // Interrupt Enable register (0xFFFF)
        uint8_t if_register = 0; // Interrupt Flag register (0xFF0F)
        std::string serial_buffer = "";
};