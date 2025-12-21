#include "ppu.h"
#include <cstdint>
#include "lcd.h"
#include "interrupts.h"
#include "cpu.h"
class LCD;

Ppu::Ppu() : bus(nullptr), lcd(nullptr)
{
    
}

void Ppu::set_cmp(Bus *bus_ptr, LCD* lcd_ptr, Cpu* cpu_ptr)
{
    bus = bus_ptr;
    lcd = lcd_ptr;
    cpu = cpu_ptr;
}

void Ppu::ppu_tick()
{
    dot++;
    switch (static_cast<LCD_Modes>(lcd->regs.lcd_status.mode_flag))
    {
        case LCD_Modes::OAM_SEARCH:
            handle_oam_search();
            break;
        case LCD_Modes::PIXEL_TRANSFER:
            handle_pixel_transfer();
            break;
        case LCD_Modes::HBLANK:
            handle_hblank();
            break;
        case LCD_Modes::VBLANK:
            handle_vblank();
            break;
    }
}

void Ppu::handle_oam_search()
{
    if (dot >= OAM_SEARCH_DOTS)
    {
        lcd->set_mode(LCD_Modes::PIXEL_TRANSFER);
    }
}

void Ppu::handle_pixel_transfer() //We handle background drawing and window drawing here
{
    if (dot >= OAM_SEARCH_DOTS + PIXEL_TRANSFER_DOTS)
    {
        lcd->set_mode(LCD_Modes::HBLANK);
    }
    
    // Get tile stage


}

void Ppu::handle_hblank()
{
    if (dot >= DOTS_PER_SCANLINE)
    {
        dot = 0;
        lcd->bump_ly();     
        if (lcd->regs.lcd_y >= VISIBLE_SCANLINES)
        {
            lcd->set_mode(LCD_Modes::VBLANK);
            cpu->request_interrupt(Interrupts::InterruptMask::IT_VBlank);
        }
        else
        {
            lcd->set_mode(LCD_Modes::OAM_SEARCH);
        }
    }
}

void Ppu::handle_vblank()
{
    if (dot >= DOTS_PER_SCANLINE)
    {
        dot = 0;
        lcd->bump_ly();        
        if (lcd->regs.lcd_y >= SCANLINES_PER_FRAME)
        {
            lcd->regs.lcd_y = 0;
            lcd->set_mode(LCD_Modes::OAM_SEARCH);
        }
    }
}

uint8_t Ppu::vram_read(uint16_t address)
{
    // VRAM range: 0x8000-0x9FFF (8KB)
    uint16_t offset = address - 0x8000;
    return reinterpret_cast<uint8_t*>(&vram)[offset];
}

void Ppu::vram_write(uint16_t address, uint8_t value)
{
    // VRAM range: 0x8000-0x9FFF (8KB)
    uint16_t offset = address - 0x8000;
    reinterpret_cast<uint8_t*>(&vram)[offset] = value;
}

uint8_t Ppu::oam_read(uint16_t address)
{
    // OAM range: 0xFE00-0xFE9F (160 bytes)
    uint8_t offset = address - 0xFE00;
    uint8_t* oam_ptr = reinterpret_cast<uint8_t*>(&oam);
    return oam_ptr[offset];
}

void Ppu::oam_write(uint16_t address, uint8_t value)
{
    // OAM range: 0xFE00-0xFE9F (160 bytes)
    uint8_t offset = address - 0xFE00;
    uint8_t* oam_ptr = reinterpret_cast<uint8_t*>(&oam);
    oam_ptr[offset] = value;
}