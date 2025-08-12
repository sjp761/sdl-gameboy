#pragma once
#include <cstdint>


class Bus
{
    public:
        Bus() = default;
        uint8_t bus_read(uint16_t address);
        void bus_write(uint16_t address, uint8_t data);
        void exram_write(uint16_t address, uint8_t value);
        void echoram_write(uint16_t address, uint8_t value);
        void wram_write(uint16_t address, uint8_t value);
        uint8_t exram_read(uint16_t address);
        uint8_t echoram_read(uint16_t address);
        uint8_t wram_read(uint16_t address);
        uint8_t eram[0x2000] = {}; // 8KB External RAM (0xA000-0xBFFF)
        uint8_t wram[0x2000] = {}; // 8KB Work RAM (0xC000-0xDFFF)
};