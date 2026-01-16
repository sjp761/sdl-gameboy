#include "mbc1.h"
#include <memory_map.h>
#include <algorithm>
#include <cstring>

void MBC1::update_banking()
{
    // Keep ROM bank register in valid range
    if (romBanks.empty()) {
        mbc1_regs.current_rom_bank = nullptr;
        return;
    }

    uint8_t bank = mbc1_regs.rom_bank_reg; // Lower 5 bits for MBC1, clamping to 1 is handled in cart_write
    bank -= 1; // Banks are 1 indexed for switchable banks (bank 1 = index 0 in romBanks, etc.)
    mbc1_regs.current_rom_bank = romBanks[bank].get();
}

MBC1::MBC1(RomData &romData) : ROM(romData)
{
    constexpr int BANK_SIZE = 0x4000; // 16 KiB per ROM bank

    // Bank 0 (0x0000-0x3FFF) is always fixed to the first 16KB of ROM, so we start loading from bank 1
    
    int total_banks = 2 << ctx.header.rom_size; // Total number of 16KB banks, rom size corresponds to bank number, see ROM_SIZES in romhelpers.h, e.g. 0x00 = 2 banks, 0x01 = 4 banks, etc.
    int switchable_banks = total_banks - 1;

    // Populate switchable banks (starting from bank 1, which is offset 0x4000 in ROM)
    for (int i = 1; i < total_banks; ++i)
    {
        auto bank = std::make_unique<uint8_t[]>(BANK_SIZE);
        int offset = i * BANK_SIZE;
        std::memcpy(bank.get(), ctx.rom_data.get() + offset, BANK_SIZE);
        romBanks.push_back(std::move(bank));
    }

    update_banking();
}

uint8_t MBC1::cart_read(uint16_t addr)
{
    if (addr >= 0x4000 && addr <= 0x7FFF) 
    {
        // Switchable ROM Bank (0x4000-0x7FFF)
        if (!romBanks.empty() && mbc1_regs.current_rom_bank) {
            return mbc1_regs.current_rom_bank[addr - 0x4000];
        }
        // Fallback for safety
        return 0xFF;
    }
    // Bank 0 (0x0000-0x3FFF) is always fixed and handled by ROM::cart_read
    return ROM::cart_read(addr);
}
void MBC1::cart_write(uint16_t addr, uint8_t value)
{
    if (MemoryMap::is_mbc1_ram_enable_area(addr)) 
    {
        // RAM Enable
        mbc1_regs.ram_enable = ((value & 0x0F) == 0x0A);
    } 
    else if (MemoryMap::is_mbc1_rom_bank_number_area(addr)) 
    {
        // ROM Bank Number (lower 5 bits)
        uint8_t bank = value & 0x1F;
        *mbc1_regs.rom_bank_reg_lower = (bank == 0) ? 1 : bank;
    } 
    else if (MemoryMap::is_mbc1_ram_bank_number_area(addr)) 
    {
        // RAM Bank Number or Upper ROM Bank bits
        mbc1_regs.ram_bank_reg = value & 0x03;
    } 
    else if (MemoryMap::is_mbc1_banking_mode_area(addr)) 
    {
        // Banking Mode Select
        mbc1_regs.banking_mode = (value & 0x01);
    }
    update_banking();
}