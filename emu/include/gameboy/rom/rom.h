#pragma once
#include <cstdint>
#include <unordered_map>
#include <string>
#include <memory>
#include "romhelpers.h"
#include "romdata.h" // For definition of RomData

class ROM
{
    public:
        ROM(RomData& romData);
        virtual uint8_t cart_read(uint16_t addr); // Will be overridden by MBC classes if ROM is MBC
        virtual void cart_write(uint16_t addr, uint8_t value); // Will be overridden by MBC classes if ROM is MBC
        cart_context ctx;
        void disable_bootrom();
    
};