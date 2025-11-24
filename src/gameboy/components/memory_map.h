#pragma once
#include <cstdint>

namespace MemoryMap {
    // Game Boy Memory Map Address Ranges
    
    // ROM Bank 00 (16KB) - Fixed
    constexpr uint16_t ROM_BANK_00_START = 0x0000;
    constexpr uint16_t ROM_BANK_00_END   = 0x3FFF;
    
    // ROM Bank 01-NN (16KB) - Switchable via MBC
    constexpr uint16_t ROM_BANK_NN_START = 0x4000;
    constexpr uint16_t ROM_BANK_NN_END   = 0x7FFF;
    
    // Video RAM (8KB)
    constexpr uint16_t VRAM_START = 0x8000;
    constexpr uint16_t VRAM_END   = 0x9FFF;
    constexpr uint16_t VRAM_SIZE  = 0x2000;
    
    // External RAM (8KB) - In Cartridge, Switchable via MBC
    constexpr uint16_t ERAM_START = 0xA000;
    constexpr uint16_t ERAM_END   = 0xBFFF;
    constexpr uint16_t ERAM_SIZE  = 0x2000;
    
    // Work RAM (8KB)
    constexpr uint16_t WRAM_START = 0xC000;
    constexpr uint16_t WRAM_END   = 0xDFFF;
    constexpr uint16_t WRAM_SIZE  = 0x2000;
    
    // Echo RAM (7.5KB) - Mirror of 0xC000-0xDDFF
    constexpr uint16_t ECHO_START = 0xE000;
    constexpr uint16_t ECHO_END   = 0xFDFF;
    
    // Object Attribute Memory (160 bytes) + I/O Registers
    constexpr uint16_t OAM_START  = 0xFE00;
    constexpr uint16_t OAM_END    = 0xFE9F;
    constexpr uint16_t IO_START   = 0xFF00;
    constexpr uint16_t IO_END     = 0xFF7F;
    constexpr uint16_t OAM_IO_START = 0xFE00;
    constexpr uint16_t OAM_IO_END   = 0xFF7F;
    constexpr uint16_t OAM_IO_SIZE  = 0x0180;
    
    // High RAM (127 bytes) - Fast internal CPU RAM
    constexpr uint16_t HRAM_START = 0xFF80;
    constexpr uint16_t HRAM_END   = 0xFFFE;
    constexpr uint16_t HRAM_SIZE  = 0x007F;
    
    // Interrupt Enable Register
    constexpr uint16_t IE_REGISTER = 0xFFFF;
    
    // Serial Port Registers
    constexpr uint16_t SERIAL_DATA = 0xFF01;      // SB - Serial transfer data
    constexpr uint16_t SERIAL_CONTROL = 0xFF02;   // SC - Serial transfer control
    
    // Helper functions to check address ranges
    inline bool is_rom(uint16_t address) {
        return address <= ROM_BANK_NN_END;
    }
    
    inline bool is_vram(uint16_t address) {
        return address >= VRAM_START && address <= VRAM_END;
    }
    
    inline bool is_eram(uint16_t address) {
        return address >= ERAM_START && address <= ERAM_END;
    }
    
    inline bool is_wram(uint16_t address) {
        return address >= WRAM_START && address <= WRAM_END;
    }
    
    inline bool is_echo(uint16_t address) {
        return address >= ECHO_START && address <= ECHO_END;
    }
    
    inline bool is_oam_io(uint16_t address) {
        return address >= OAM_IO_START && address <= OAM_IO_END;
    }
    
    inline bool is_hram(uint16_t address) {
        return address >= HRAM_START && address <= IE_REGISTER;
    }
}
