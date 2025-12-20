#include "lcd.h"
#include "interrupts.h"
#include "cpu.h"

LCD::LCD() : regs{}, ppu(nullptr)
{
}

void LCD::set_cmp(Ppu *ppu_ptr, Cpu* cpu_ptr)
{
    ppu = ppu_ptr;
    cpu = cpu_ptr;
}

void LCD::lcd_write(uint16_t addr, uint8_t value)
{
    *(reinterpret_cast<uint8_t*>(&regs) + (addr - 0xFF40)) = value; //FF40 is the starting address of LCD registers
}

uint8_t LCD::lcd_read(uint16_t addr)
{
    return *(reinterpret_cast<uint8_t*>(&regs) + (addr - 0xFF40));
}

void LCD::bump_ly()
{
    regs.lcd_y++;
    if (regs.lcd_y == regs.lcd_y_compare)
    {
        regs.lcd_status |= static_cast<uint8_t>(LCD_Status_Flags::LYC_EQ_LY_FLAG);
        if (regs.lcd_status & static_cast<uint8_t>(LCD_Status_Flags::LYC_EQ_LY_INTERRUPT))
        {
            cpu->request_interrupt(Interrupts::InterruptMask::IT_LCDStat);
        }
    }
    else
    {
        regs.lcd_status &= ~static_cast<uint8_t>(LCD_Status_Flags::LYC_EQ_LY_FLAG);
    }
}

void LCD::set_mode(LCD_Modes mode)
{
    regs.lcd_status = (regs.lcd_status & 0b11111100) | static_cast<uint8_t>(mode);
}