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
    switch(addr)
    {
        case 0xFF40: // LCD Control
            byte_to_lcd_control(regs.lcd_control, value);
            break;
        case 0xFF41: // LCD Status
            byte_to_lcd_status(regs.lcd_status, value);
            break;
        default:
            // For other registers, use the old method
            *(reinterpret_cast<uint8_t*>(&regs) + (addr - 0xFF40)) = value;
            break;
    }
}

uint8_t LCD::lcd_read(uint16_t addr)
{
    switch(addr)
    {
        case 0xFF40: // LCD Control
            return lcd_control_to_byte(regs.lcd_control);
        case 0xFF41: // LCD Status
            return lcd_status_to_byte(regs.lcd_status);
        default:
            // For other registers, use the old method
            return *(reinterpret_cast<uint8_t*>(&regs) + (addr - 0xFF40));
    }
}

void LCD::bump_ly()
{
    regs.lcd_y++;
    check_lyc();
}

void LCD::set_mode(LCD_Modes mode)
{
    regs.lcd_status.mode_flag = static_cast<uint8_t>(mode);
}

uint8_t LCD::get_lcd_control_attr(lcd_control_bits bit)
{
    uint8_t ctrl_byte = lcd_control_to_byte(regs.lcd_control);
    return (ctrl_byte >> static_cast<uint8_t>(bit)) & 1;
}

uint8_t LCD::get_lcd_status_attr(lcd_status_bits bit)
{
    uint8_t status_byte = lcd_status_to_byte(regs.lcd_status);
    return (status_byte >> static_cast<uint8_t>(bit)) & 1;
}

void LCD::check_lyc()
{
    if (regs.lcd_y == regs.lcd_y_compare)
    {
        regs.lcd_status.lyc_eq_ly_flag = 1;
        if (regs.lcd_status.lyc_eq_ly_interrupt)
        {
            cpu->request_interrupt(Interrupts::InterruptMask::IT_LCDStat);
        }
    }
    else
    {
        regs.lcd_status.lyc_eq_ly_flag = 0;
    }
}
