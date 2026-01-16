#pragma once
#include <cstdint>
#include <memory>
#include <vector>
#include "rom.h"
#include "romdata.h"


struct mbc1_registers
{
    bool ram_enable = false; // RAM Enable (0x0000-0x1FFF)
    uint8_t rom_bank_reg = 1; // ROM Bank Number (0x2000-0x3FFF)
    uint8_t ram_bank_reg = 0; // RAM Bank Number (0x4000-0x5FFF) - this can also be used for upper rom bank number bits
    bool banking_mode = false; // Banking Mode Select (0x6000-0x7FFF)
    uint8_t* rom_bank_reg_lower = &rom_bank_reg; // Lower 5 bits of ROM bank number - these aliases are to clarify usage
    uint8_t* rom_bank_reg_upper = &ram_bank_reg; // Upper 2 bits of ROM bank number when in ROM banking mode

    uint8_t* current_ram_bank = nullptr; // Pointer to the currently selected RAM bank
    uint8_t* current_rom_bank = nullptr; // Currently selected ROM bank after applying banking mode logic

};

class MBC1 : public ROM
{
    private:
        mbc1_registers mbc1_regs;
        void update_banking();
        std::vector<std::unique_ptr<uint8_t[]>> romBanks;
    public:
        MBC1(RomData& romData);
        uint8_t cart_read(uint16_t addr) override;
        void cart_write(uint16_t addr, uint8_t value) override;
        
};