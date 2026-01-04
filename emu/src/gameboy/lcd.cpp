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
            check_lyc();
            break;
        case 0xFF42: // SCY
            regs.scroll_y = value;
            break;
        case 0xFF43: // SCX
            regs.scroll_x = value;
            break;
        case 0xFF44: // LY
            regs.lcd_y = 0; // Writing to LY resets it to 0
            check_lyc(); // Check interrupts after reset
            break;
        case 0xFF45: // LYC
            regs.lcd_y_compare = value;
            check_lyc();
            break;
        case 0xFF46: // DMA handled in bus component
            break;
        case 0xFF47: // BGP
            regs.bg_palette = value;
            break;
        case 0xFF48: // OBP0
            regs.obj_palette_0 = value;
            break;
        case 0xFF49: // OBP1
            regs.obj_palette_1 = value;
            break;
        case 0xFF4A: // WY
            regs.window_y = value;
            break;
        case 0xFF4B: // WX
            regs.window_x = value;
            break;
        default:
            // Ignore invalid writes
            break;
    }
}

uint8_t LCD::lcd_read(uint16_t addr) const
{
    switch(addr)
    {
        case 0xFF40: // LCD Control
            return lcd_control_to_byte(regs.lcd_control);
        case 0xFF41: // LCD Status
            return lcd_status_to_byte(regs.lcd_status);
        case 0xFF42: // SCY
            return regs.scroll_y;
        case 0xFF43: // SCX
            return regs.scroll_x;
        case 0xFF44: // LY
            return regs.lcd_y;
        case 0xFF45: // LYC
            return regs.lcd_y_compare;
        case 0xFF46: // DMA
            return 0xFF; // Not readable from LCD usually
        case 0xFF47: // BGP
            return regs.bg_palette;
        case 0xFF48: // OBP0
            return regs.obj_palette_0;
        case 0xFF49: // OBP1
            return regs.obj_palette_1;
        case 0xFF4A: // WY
            return regs.window_y;
        case 0xFF4B: // WX
            return regs.window_x;
        default:
            return 0xFF;
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

uint8_t LCD::get_lcd_control_attr(lcd_control_bits bit) const
{
    uint8_t ctrl_byte = lcd_control_to_byte(regs.lcd_control);
    return (ctrl_byte >> static_cast<uint8_t>(bit)) & 1;
}

uint8_t LCD::get_lcd_status_attr(lcd_status_bits bit) const
{
    uint8_t status_byte = lcd_status_to_byte(regs.lcd_status);
    return (status_byte >> static_cast<uint8_t>(bit)) & 1;
}

void LCD::check_lyc()
{
    bool match = (regs.lcd_y == regs.lcd_y_compare);
    regs.lcd_status.lyc_eq_ly_flag = match;
    if (match && regs.lcd_status.lyc_eq_ly_interrupt)
    {
        cpu->request_interrupt(Interrupts::InterruptMask::IT_LCDStat);
    }
}
