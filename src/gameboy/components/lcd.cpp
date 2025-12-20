#include "lcd.h"

LCD::LCD() : regs{}, ppu(nullptr)
{
}

void LCD::set_cmp(Ppu *ppu_ptr)
{
    ppu = ppu_ptr;
}

void LCD::lcd_write(uint16_t addr, uint8_t value)
{
    *(reinterpret_cast<uint8_t*>(&regs) + (addr - 0xFF40)) = value; //FF40 is the starting address of LCD registers
}

uint8_t LCD::lcd_read(uint16_t addr)
{
    return *(reinterpret_cast<uint8_t*>(&regs) + (addr - 0xFF40));
}
