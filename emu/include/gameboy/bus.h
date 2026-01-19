#pragma once
#include <cstdint>
#include "memory_map.h"
#include <string>
#include <array>
#include <memory>

class Timer; // Forward declaration
class ROM; // Forward declaration
class Ppu; // Forward declaration
class DMA;
class LCD;
class Bus
{
    private:
        ROM* rom;
        Timer* timer; // Timer reference
        Ppu* ppu; //  PPU reference
        DMA* dma; //  DMA reference
        LCD* lcd; //  LCD reference
        
        // Table-driven memory region dispatch
        struct MemoryRegion {
            uint16_t start;
            uint16_t end;
            uint8_t (Bus::*read_fn)(uint16_t);
            void (Bus::*write_fn)(uint16_t, uint8_t);
        };
        
        static constexpr size_t NUM_REGIONS = 9;
        std::array<MemoryRegion, NUM_REGIONS> memory_regions;
        
        void init_memory_table();
        uint8_t rom_read(uint16_t address);
        void rom_write(uint16_t address, uint8_t value);
        uint8_t hram_read(uint16_t address);
        void hram_write(uint16_t address, uint8_t value);
        uint8_t lcd_read(uint16_t address);
        void lcd_write(uint16_t address, uint8_t value);
        uint8_t audio_read(uint16_t address);
        void audio_write(uint16_t address, uint8_t value);
        
    public:
        Bus();
        // Set component pointers
        void set_cmp(ROM* rom_ptr, Timer* timer_ptr, Ppu* ppu_ptr, DMA* dma_ptr, LCD* lcd_ptr) { rom = rom_ptr; timer = timer_ptr; ppu = ppu_ptr; dma = dma_ptr; lcd = lcd_ptr; }
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
        uint8_t audio_regs[MemoryMap::AUDIO_SIZE] = {}; // Audio registers (0xFF10-0xFF26)
        uint8_t wave_ram[MemoryMap::WAVE_RAM_SIZE] = {}; // Wave Pattern RAM (0xFF30-0xFF3F)
        std::string serial_buffer = "";

        std::unique_ptr<uint8_t[]> opcode_test_mem = std::make_unique<uint8_t[]>(64*1024); // 64KB flat memory for opcode tests
        bool test_mode = false; // Flag to indicate if in test mode
        Bus(bool test_mode_enable);

        #ifdef OPCODE_TEST
            uint8_t opcode_test_memory[64 * 1024] = {}; // 64KB flat memory for opcode tests
        #endif
};